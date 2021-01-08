#include "AdultDvdEmpireApi.h"

#include "globals/Meta.h"
#include "network/NetworkRequest.h"

namespace mediaelch {
namespace scraper {

AdultDvdEmpireApi::AdultDvdEmpireApi(QObject* parent) : QObject(parent)
{
}

void AdultDvdEmpireApi::sendGetRequest(const QUrl& url, AdultDvdEmpireApi::ApiCallback callback)
{
    if (m_cache.hasValidElement(url, Locale::English)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, [cb = std::move(callback), element = m_cache.getElement(url, Locale::English)]() { //
            cb(element, {});
        });
        return;
    }

    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    // Some server-side detection if the browser supports hovering or certain JavaScript features.
    // If we use the MediaElch user agent, then no actor images are sent in the response (i.e. HTML).
    // See GitHub issue #1164
    mediaelch::network::useFirefoxUserAgent(request);
    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qWarning() << "[AdultDvdEmpireApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        if (!data.isEmpty()) {
            m_cache.addElement(reply->url(), Locale::English, data);
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

void AdultDvdEmpireApi::searchForMovie(const QString& query, AdultDvdEmpireApi::ApiCallback callback)
{
    sendGetRequest(makeMovieSearchUrl(query), std::move(callback));
}

void AdultDvdEmpireApi::loadMovie(const QString& id, AdultDvdEmpireApi::ApiCallback callback)
{
    sendGetRequest(makeMovieUrl(id), std::move(callback));
}

QUrl AdultDvdEmpireApi::makeApiUrl(const QString& suffix, QUrlQuery query) const
{
    return QStringLiteral("https://www.adultdvdempire.com%1?%2").arg(suffix, query.toString());
}

QUrl AdultDvdEmpireApi::makeMovieSearchUrl(const QString& searchStr) const
{
    QUrlQuery queries;
    queries.addQueryItem("view", "list");
    queries.addQueryItem("q", searchStr);
    return makeApiUrl("/allsearch/search", queries);
}

QUrl AdultDvdEmpireApi::makeMovieUrl(const QString& id) const
{
    return makeApiUrl(id, {});
}

} // namespace scraper
} // namespace mediaelch
