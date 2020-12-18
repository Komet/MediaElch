#include "ImdbTvApi.h"

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

ImdbTvApi::ImdbTvApi(QObject* parent) : QObject(parent)
{
}

void ImdbTvApi::initialize()
{
    // no-op
}

bool ImdbTvApi::isInitialized() const
{
    return true;
}

void ImdbTvApi::sendGetRequest(const Locale& locale, const QUrl& url, ImdbTvApi::ApiCallback callback)
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

    connect(reply, &QNetworkReply::finished, [reply, cb = std::move(callback), locale, this]() {
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

void ImdbTvApi::searchForShow(const Locale& locale, const QString& query, ImdbTvApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowSearchUrl(query), std::move(callback));
}

void ImdbTvApi::loadDefaultEpisodesPage(const Locale& locale, const ImdbId& showId, ImdbTvApi::ApiCallback callback)
{
    sendGetRequest(locale, getDefaultEpisodesUrl(showId), callback);
}

void ImdbTvApi::loadShowInfos(const Locale& locale, const ImdbId& showId, ImdbTvApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowUrl(showId), callback);
}

void ImdbTvApi::loadSeason(const Locale& locale,
    const ImdbId& showId,
    SeasonNumber season,
    ImdbTvApi::ApiCallback callback)
{
    sendGetRequest(locale, getSeasonUrl(showId, season), callback);
}

void ImdbTvApi::loadEpisode(const Locale& locale, const ImdbId& episodeId, ApiCallback callback)
{
    sendGetRequest(locale, getEpisodeUrl(episodeId), callback);
}

void ImdbTvApi::addHeadersToRequest(const Locale& locale, QNetworkRequest& request)
{
    request.setRawHeader("Accept-Language", locale.toString('-').toLocal8Bit());
}

QUrl ImdbTvApi::makeFullUrl(const QString& suffix)
{
    return QUrl("https://www.imdb.com" + suffix);
}

QUrl ImdbTvApi::makeFullAssetUrl(const QString& suffix)
{
    return QUrl("https://www.imdb.com" + suffix);
}

QUrl ImdbTvApi::getShowSearchUrl(const QString& searchStr) const
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

QUrl ImdbTvApi::getShowUrl(const ImdbId& id) const
{
    return makeFullUrl(QStringLiteral("/title/") + id.toString() + '/');
}

QUrl ImdbTvApi::getEpisodeUrl(const ImdbId& episodeId) const
{
    // All episodes also have an unique ID.
    return makeFullUrl(QStringLiteral("/title/") + episodeId.toString() + '/');
}

QUrl ImdbTvApi::getSeasonUrl(const ImdbId& showId, SeasonNumber season) const
{
    QUrlQuery queries;
    queries.addQueryItem("season", season.toString());
    return makeFullUrl(QStringLiteral("/title/") + showId.toString() + //
                       QStringLiteral("/episodes?") + queries.toString());
}

QUrl ImdbTvApi::getDefaultEpisodesUrl(const ImdbId& showId) const
{
    return makeFullUrl(QStringLiteral("/title/") + showId.toString() + QStringLiteral("/episodes"));
}


} // namespace scraper
} // namespace mediaelch
