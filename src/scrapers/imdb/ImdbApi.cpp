#include "ImdbApi.h"

#include "Version.h"
#include "globals/JsonRequest.h"
#include "globals/Meta.h"
#include "network/NetworkRequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace mediaelch {
namespace scraper {

ImdbApi::ImdbApi(QObject* parent) : QObject(parent)
{
}

void ImdbApi::initialize()
{
    // no-op
}

bool ImdbApi::isInitialized() const
{
    return true;
}

void ImdbApi::sendGetRequest(const Locale& locale, const QUrl& url, ImdbApi::ApiCallback callback)
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
    addHeadersToRequest(locale, request);

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), locale, this]() {
        auto dls = makeDeleteLaterScope(reply);
        QString html;
        if (reply->error() == QNetworkReply::NoError) {
            html = QString::fromUtf8(reply->readAll());

            if (!html.isEmpty()) {
                m_cache.addElement(reply->url(), locale, html);
            }
        } else {
            qWarning() << "[ImdbTv][Api] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        ScraperError error = makeScraperError(html, *reply, {});
        cb(html, error);
    });
}

void ImdbApi::searchForShow(const Locale& locale, const QString& query, ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowSearchUrl(query), std::move(callback));
}

void ImdbApi::searchForMovie(const Locale& locale,
    const QString& query,
    bool includeAdult,
    ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, makeMovieSearchUrl(query, includeAdult), std::move(callback));
}

void mediaelch::scraper::ImdbApi::loadMovie(const Locale& locale, const ImdbId& movieId, ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, makeMovieUrl(movieId), callback);
}

void ImdbApi::loadDefaultEpisodesPage(const Locale& locale, const ImdbId& showId, ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, getDefaultEpisodesUrl(showId), callback);
}

void ImdbApi::loadShowInfos(const Locale& locale, const ImdbId& showId, ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowUrl(showId), callback);
}

void ImdbApi::loadSeason(const Locale& locale, const ImdbId& showId, SeasonNumber season, ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, getSeasonUrl(showId, season), callback);
}

void ImdbApi::loadEpisode(const Locale& locale, const ImdbId& episodeId, ApiCallback callback)
{
    sendGetRequest(locale, getEpisodeUrl(episodeId), callback);
}

void ImdbApi::addHeadersToRequest(const Locale& locale, QNetworkRequest& request)
{
    request.setRawHeader("Accept-Language", locale.toString('-').toLocal8Bit());
}

QUrl ImdbApi::makeMovieUrl(const ImdbId& id) const
{
    return makeFullUrl(QStringLiteral("/title/%1/").arg(id.toString()));
}

QUrl ImdbApi::makeMovieSearchUrl(const QString& searchStr, bool includeAdult) const
{
    if (includeAdult) {
        QUrlQuery queries;
        queries.addQueryItem("adult", "include");
        queries.addQueryItem("title_type", "feature,documentary,tv_movie,short,video"); // Movie categories
        queries.addQueryItem("view", "simple");
        queries.addQueryItem("count", "100");
        queries.addQueryItem("title", searchStr);
        return makeFullUrl("/search/title/?" + queries.toString());
    }
    QUrlQuery queries;
    queries.addQueryItem("s", "tt");
    queries.addQueryItem("ttype", "ft"); // Movie category
    queries.addQueryItem("ref_", "fn_ft");
    queries.addQueryItem("q", searchStr);
    return makeFullUrl("/find?" + queries.toString());
}

QUrl ImdbApi::makeFullUrl(const QString& suffix)
{
    return QUrl("https://www.imdb.com" + suffix);
}

QUrl ImdbApi::makeFullAssetUrl(const QString& suffix)
{
    return QUrl("https://www.imdb.com" + suffix);
}

QUrl ImdbApi::getShowSearchUrl(const QString& searchStr) const
{
    if (ImdbId::isValidFormat(searchStr)) {
        return makeFullUrl(QStringLiteral("/title/") + searchStr + '/');
    }

    QUrlQuery queries;
    queries.addQueryItem("s", "tt");
    queries.addQueryItem("ttype", "tv"); // TV category
    queries.addQueryItem("q", searchStr);
    return makeFullUrl("/find?" + queries.toString());
}

QUrl ImdbApi::getShowUrl(const ImdbId& id) const
{
    return makeFullUrl(QStringLiteral("/title/") + id.toString() + '/');
}

QUrl ImdbApi::getEpisodeUrl(const ImdbId& episodeId) const
{
    // All episodes also have an unique ID.
    return makeFullUrl(QStringLiteral("/title/") + episodeId.toString() + '/');
}

QUrl ImdbApi::getSeasonUrl(const ImdbId& showId, SeasonNumber season) const
{
    QUrlQuery queries;
    queries.addQueryItem("season", season.toString());
    return makeFullUrl(QStringLiteral("/title/") + showId.toString() + //
                       QStringLiteral("/episodes?") + queries.toString());
}

QUrl ImdbApi::getDefaultEpisodesUrl(const ImdbId& showId) const
{
    return makeFullUrl(QStringLiteral("/title/") + showId.toString() + QStringLiteral("/episodes"));
}


} // namespace scraper
} // namespace mediaelch
