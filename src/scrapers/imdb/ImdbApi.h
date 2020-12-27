#pragma once

#include "data/ImdbId.h"
#include "data/Locale.h"
#include "globals/ScraperInfos.h"
#include "network/NetworkManager.h"
#include "network/WebsiteCache.h"
#include "scrapers/ScraperError.h"
#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/SeasonOrder.h"

#include <QByteArray>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

#include <functional>

namespace mediaelch {
namespace scraper {

/// \brief API interface for TheTvDb
class ImdbApi : public QObject
{
    Q_OBJECT

public:
    explicit ImdbApi(QObject* parent = nullptr);
    ~ImdbApi() override = default;

    void initialize();
    bool isInitialized() const;

public:
    using ApiCallback = std::function<void(QString, ScraperError)>;

    void sendGetRequest(const Locale& locale, const QUrl& url, ApiCallback callback);

    void searchForMovie(const Locale& locale, const QString& query, bool includeAdult, ApiCallback callback);
    void searchForShow(const Locale& locale, const QString& query, ApiCallback callback);

    void loadMovie(const Locale& locale, const ImdbId& movieId, ApiCallback callback);

    void loadDefaultEpisodesPage(const Locale& locale, const ImdbId& showId, ApiCallback callback);

    void loadShowInfos(const Locale& locale, const ImdbId& showId, ApiCallback callback);
    void loadSeason(const Locale& locale, const ImdbId& showId, SeasonNumber season, ApiCallback callback);
    void loadEpisode(const Locale& locale, const ImdbId& episodeId, ApiCallback callback);

signals:
    void initialized();

public:
    static QUrl makeFullUrl(const QString& suffix);
    static QUrl makeFullAssetUrl(const QString& suffix);

private:
    /// \brief Add neccassaray headers for IMDb to the request object.
    void addHeadersToRequest(const Locale& locale, QNetworkRequest& request);

    QUrl makeMovieUrl(const ImdbId& id) const;
    QUrl makeMovieSearchUrl(const QString& searchStr, bool includeAdult) const;

    QUrl getShowUrl(const ImdbId& id) const;
    QUrl getShowSearchUrl(const QString& searchStr) const;
    QUrl getEpisodeUrl(const ImdbId& episodeId) const;
    QUrl getSeasonUrl(const ImdbId& showId, SeasonNumber season) const;
    QUrl getDefaultEpisodesUrl(const ImdbId& showId) const;

private:
    const QString m_language;
    mediaelch::network::NetworkManager m_network;
    WebsiteCache m_cache;
};

} // namespace scraper
} // namespace mediaelch
