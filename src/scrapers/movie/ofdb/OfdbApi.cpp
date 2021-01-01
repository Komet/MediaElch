#include "OfdbApi.h"

#include "globals/Helper.h"
#include "globals/Meta.h"
#include "network/NetworkRequest.h"

namespace mediaelch {
namespace scraper {

OfdbApi::OfdbApi(QObject* parent) : QObject(parent)
{
}

void OfdbApi::sendGetRequest(const QUrl& url, OfdbApi::ApiCallback callback)
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
    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qWarning() << "[OfdbApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        if (!data.isEmpty()) {
            m_cache.addElement(reply->url(), Locale::English, data);
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

void OfdbApi::searchForMovie(const QString& query, OfdbApi::ApiCallback callback)
{
    sendGetRequest(makeMovieSearchUrl(query), std::move(callback));
}

void OfdbApi::loadMovie(const QString& id, OfdbApi::ApiCallback callback)
{
    sendGetRequest(makeMovieUrl(id), std::move(callback));
}

QUrl OfdbApi::makeApiUrl(const QString& suffix, QUrlQuery query) const
{
    return QStringLiteral("http://ofdbgw.metawave.ch%1?%2").arg(suffix, query.toString());
}

QUrl OfdbApi::makeMovieSearchUrl(const QString& searchStr) const
{
    QString encodedSearch = helper::toLatin1PercentEncoding(searchStr);
    return makeApiUrl(QStringLiteral("/search/%1").arg(encodedSearch), {});
}

QUrl OfdbApi::makeMovieUrl(const QString& id) const
{
    // TODO QString encodedSearch = helper::toLatin1PercentEncoding(searchStr);
    return makeApiUrl(QStringLiteral("/search/%1").arg(id), {});
}

} // namespace scraper
} // namespace mediaelch
