#include "AebnApi.h"

#include "globals/Meta.h"
#include "network/NetworkRequest.h"

namespace mediaelch {
namespace scraper {

AebnApi::AebnApi(QObject* parent) : QObject(parent)
{
}

void AebnApi::sendGetRequest(const QUrl& url, const Locale& locale, AebnApi::ApiCallback callback)
{
    if (m_cache.hasValidElement(url, locale)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, [cb = std::move(callback), element = m_cache.getElement(url, locale)]() { //
            cb(element, {});
        });
        return;
    }

    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), locale, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qWarning() << "[AebnApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        if (!data.isEmpty()) {
            m_cache.addElement(reply->url(), locale, data);
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

void AebnApi::searchForMovie(const QString& query,
    const Locale& locale,
    const QString& genre,
    AebnApi::ApiCallback callback)
{
    sendGetRequest(makeMovieSearchUrl(query, genre, locale), locale, std::move(callback));
}

void AebnApi::loadMovie(const QString& id, const Locale& locale, const QString& genre, AebnApi::ApiCallback callback)
{
    sendGetRequest(makeMovieUrl(id, genre, locale), locale, std::move(callback));
}

void AebnApi::loadActor(const QString& id, const Locale& locale, const QString& genre, AebnApi::ApiCallback callback)
{
    sendGetRequest(makeActorUrl(id, genre, locale), locale, std::move(callback));
}

QUrl AebnApi::makeApiUrl(const QString& suffix, const Locale& locale, QUrlQuery query) const
{
    query.addQueryItem("locale", locale.toString());
    return QStringLiteral("https://straight.theater.aebn.net%1?%2").arg(suffix, query.toString());
}

QUrl AebnApi::makeMovieSearchUrl(const QString& searchStr, const QString& genre, const Locale& locale) const
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrlQuery queries;
    queries.addQueryItem("userQuery", encodedSearch);
    queries.addQueryItem("targetSearchMode", "basic");
    queries.addQueryItem("searchType", "movie");
    queries.addQueryItem("sortType", "Relevance");
    queries.addQueryItem("imageType", "Large");
    queries.addQueryItem("theaterId", "822");
    queries.addQueryItem("genreId", genre);
    return makeApiUrl("/dispatcher/fts", locale, queries);
}

QUrl AebnApi::makeMovieUrl(const QString& id, const QString& genre, const Locale& locale) const
{
    QUrlQuery queries;
    queries.addQueryItem("movieId", id);
    queries.addQueryItem("theaterId", "822");
    queries.addQueryItem("genreId", genre);
    return makeApiUrl("/dispatcher/movieDetail", locale, queries);
}

QUrl AebnApi::makeActorUrl(const QString& id, const QString& genre, const Locale& locale) const
{
    QUrlQuery queries;
    queries.addQueryItem("starId", id);
    queries.addQueryItem("theaterId", "822");
    queries.addQueryItem("genreId", genre);
    return makeApiUrl("/dispatcher/starDetail", locale, queries);
}

} // namespace scraper
} // namespace mediaelch
