#include "HotMoviesApi.h"

#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace scraper {

HotMoviesApi::HotMoviesApi(QObject* parent) : QObject(parent)
{
}

void HotMoviesApi::sendGetRequest(const QUrl& url, HotMoviesApi::ApiCallback callback)
{
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

    if (m_network.cache().hasValidElement(request)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, this, [cb = std::move(callback), element = m_network.cache().getElement(request)]() { //
            cb(element, {});
        });
        return;
    }

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), request, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qCWarning(generic) << "[HotMoviesApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        if (!data.isEmpty()) {
            m_network.cache().addElement(request, data);
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

void HotMoviesApi::searchForMovie(const QString& query, HotMoviesApi::ApiCallback callback)
{
    sendGetRequest(makeMovieSearchUrl(query), std::move(callback));
}

void HotMoviesApi::loadMovie(const QString& id, HotMoviesApi::ApiCallback callback)
{
    sendGetRequest(makeMovieUrl(id), std::move(callback));
}

QUrl HotMoviesApi::makeApiUrl(const QString& suffix, QUrlQuery query) const
{
    return QStringLiteral("https://www.hotmovies.com%1?%2").arg(suffix, query.toString());
}

QUrl HotMoviesApi::makeMovieSearchUrl(const QString& searchStr) const
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrlQuery queries;
    queries.addQueryItem("q", encodedSearch);
    QUrl url = makeApiUrl("/adult-movies/search", queries);
    return url;
}

QUrl HotMoviesApi::makeMovieUrl(const QString& id) const
{
    return id; // the id is actually an URL
}

} // namespace scraper
} // namespace mediaelch
