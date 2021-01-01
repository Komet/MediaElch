#include "VideoBusterApi.h"

#include "globals/Helper.h"
#include "globals/Meta.h"
#include "network/NetworkRequest.h"

namespace mediaelch {
namespace scraper {

VideoBusterApi::VideoBusterApi(QObject* parent) : QObject(parent)
{
}

void VideoBusterApi::sendGetRequest(const QUrl& url, VideoBusterApi::ApiCallback callback)
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
            qWarning() << "[VideoBusterApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        if (!data.isEmpty()) {
            m_cache.addElement(reply->url(), Locale::English, data);
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

void VideoBusterApi::searchForMovie(const QString& query, VideoBusterApi::ApiCallback callback)
{
    sendGetRequest(makeMovieSearchUrl(query), std::move(callback));
}

void VideoBusterApi::loadMovie(const QString& id, VideoBusterApi::ApiCallback callback)
{
    sendGetRequest(makeMovieUrl(id), std::move(callback));
}


QUrl VideoBusterApi::makeApiUrl(const QString& suffix, QUrlQuery query) const
{
    return QStringLiteral("https://www.videobuster.de%1?%2").arg(suffix, query.toString());
}

QUrl VideoBusterApi::makeMovieSearchUrl(const QString& searchStr) const
{
    QString encodedSearch = helper::toLatin1PercentEncoding(searchStr);
    QUrlQuery queries;
    queries.addQueryItem("tab_search_content", "movies");
    queries.addQueryItem("view", "title_list_view_option_list");
    queries.addQueryItem("search_title", encodedSearch);
    return makeApiUrl("/titlesearch.php", queries);
}

QUrl VideoBusterApi::makeMovieUrl(const QString& id) const
{
    return makeApiUrl(id, {});
}

} // namespace scraper
} // namespace mediaelch
