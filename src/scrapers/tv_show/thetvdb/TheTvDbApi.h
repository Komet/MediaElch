#pragma once

#include "data/Locale.h"
#include "globals/ScraperInfos.h"
#include "network/NetworkManager.h"
#include "network/WebsiteCache.h"
#include "scrapers/ScraperError.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/SeasonOrder.h"
#include "tv_shows/TvDbId.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

#include <functional>

namespace mediaelch {
namespace scraper {

/// \brief API interface for TheTvDb
class TheTvDbApi : public QObject
{
    Q_OBJECT

public:
    explicit TheTvDbApi(QObject* parent = nullptr);
    ~TheTvDbApi() override = default;

    void initialize();
    bool isInitialized() const;

public:
    using ApiPage = int;

    struct Paginate
    {
        ApiPage first{0};
        ApiPage last{0};
        ApiPage next{0};
        ApiPage prev{0};

        bool hasNextPage() const { return next > 0; }
    };

public:
    using ApiCallback = std::function<void(QJsonDocument, ScraperError)>;

    void sendGetRequest(const Locale& locale, const QUrl& url, ApiCallback callback);

    void searchForShow(const Locale& locale, const QString& query, ApiCallback callback);
    void loadShowInfos(const Locale& locale, const TvDbId& id, ApiCallback callback);
    void loadShowActors(const Locale& locale, const TvDbId& id, ApiCallback callback);
    void loadImageUrls(const Locale& locale, const TvDbId& id, ShowScraperInfo imageType, ApiCallback callback);

    void
    loadSeason(const Locale& locale, const TvDbId& id, SeasonNumber season, SeasonOrder order, ApiCallback callback);
    void loadSeasonsPage(const Locale& locale,
        const TvDbId& id,
        const QSet<SeasonNumber>& seasons,
        SeasonOrder order,
        ApiPage page,
        ApiCallback callback);
    void
    loadAllSeasonsPage(const Locale& locale, const TvDbId& id, SeasonOrder order, ApiPage page, ApiCallback callback);

    void loadEpisode(const Locale& locale, const TvDbId& episodeId, ApiCallback callback);

signals:
    void initialized(bool wasSuccessful);

public:
    /// \brief An ApiToken represents an API token from TheTvDb.
    ///
    /// These tokens are JSON web tokens and are valid for about 24h
    /// (according to https://api.thetvdb.com/swagger).
    class ApiToken
    {
    public:
        ApiToken() = default;
        explicit ApiToken(QString token) : m_token(std::move(token)) {}
        bool isValid() const { return !m_token.isEmpty(); }
        QByteArray toBearer() const { return "Bearer " + m_token.toLocal8Bit(); }

    private:
        QString m_token;
    };

    enum class ApiShowDetails
    {
        INFOS,
        ACTORS
    };

public:
    static QUrl makeFullUrl(const QString& suffix);
    static QUrl makeFullAssetUrl(const QString& suffix);

private:
    /// \brief Add neccassaray headers for TheTvDb to the request object.
    /// Token must exist.
    /// \see TheTvDbApi::obtainJsonWebToken
    void addHeadersToRequest(const Locale& locale, QNetworkRequest& request);


    QUrl getShowUrl(ApiShowDetails type, const TvDbId& id) const;
    QUrl getShowSearchUrl(const QString& searchStr) const;
    QUrl getImagesUrl(ShowScraperInfo type, const TvDbId& id) const;
    QUrl getEpisodesUrl(const TvDbId& showId, ApiPage page) const;
    QUrl getSeasonUrl(const TvDbId& showId, SeasonNumber season, SeasonOrder order) const;
    QUrl getEpisodeUrl(const TvDbId& episodeId) const;
    QString seasonOrderToUrlArg(SeasonOrder order) const;

private:
    const QString m_language;
    mediaelch::network::NetworkManager m_network;
    ApiToken m_token;
    WebsiteCache m_cache;
};

} // namespace scraper
} // namespace mediaelch
