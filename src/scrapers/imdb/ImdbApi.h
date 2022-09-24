#pragma once

#include "data/ImdbId.h"
#include "data/Locale.h"
#include "data/tv_show/EpisodeNumber.h"
#include "data/tv_show/SeasonNumber.h"
#include "data/tv_show/SeasonOrder.h"
#include "network/NetworkManager.h"
#include "network/WebsiteCache.h"
#include "scrapers/ScraperError.h"
#include "scrapers/ScraperInfos.h"

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
    /// \brief What detail page of a movie should be loaded.
    enum class PageKind
    {
        Main,
        Reference,
        PlotSummary,
        ReleaseInfo,
        Keywords,
    };

public:
    using ApiCallback = std::function<void(QString, ScraperError)>;

    void sendGetRequest(const Locale& locale, const QUrl& url, ApiCallback callback);

    void searchForMovie(const Locale& locale, const QString& query, bool includeAdult, ApiCallback callback);
    void searchForShow(const Locale& locale, const QString& query, ApiCallback callback);

    void loadTitle(const Locale& locale, const ImdbId& movieId, PageKind page, ApiCallback callback);

    void loadDefaultEpisodesPage(const Locale& locale, const ImdbId& showId, ApiCallback callback);

    void loadSeason(const Locale& locale, const ImdbId& showId, SeasonNumber season, ApiCallback callback);

signals:
    void initialized();

public:
    static QUrl makeFullUrl(const QString& suffix);
    static QUrl makeFullAssetUrl(const QString& suffix);

private:
    /// \brief Add neccassaray headers for IMDb to the request object.
    void addHeadersToRequest(const Locale& locale, QNetworkRequest& request);

    QUrl makeTitleUrl(const ImdbId& id, PageKind page) const;
    QUrl makeMovieSearchUrl(const QString& searchStr, bool includeAdult) const;
    QUrl makeShowSearchUrl(const QString& searchStr) const;
    QUrl makeSeasonUrl(const ImdbId& showId, SeasonNumber season) const;
    QUrl makeDefaultEpisodesUrl(const ImdbId& showId) const;

private:
    const QString m_language;
    mediaelch::network::NetworkManager m_network;
    WebsiteCache m_cache;
};

} // namespace scraper
} // namespace mediaelch
