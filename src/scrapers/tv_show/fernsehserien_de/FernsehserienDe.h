#pragma once

#include "network/NetworkManager.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/fernsehserien_de/FernsehserienDeConfiguration.h"

namespace mediaelch {
namespace scraper {

class FernsehserienDeApi
{
public:
    QUrl searchUrl(const QString& query);
    QUrl tvShowUrl(const ShowIdentifier& id);
    QUrl episodeUrl(const QString& episodeId);
    QUrl seasonsOverviewUrl(const ShowIdentifier& id);

    mediaelch::network::NetworkManager& network() { return m_network; }

private:
    mediaelch::network::NetworkManager m_network;
};


class FernsehserienDe : public TvScraper
{
    Q_OBJECT

public:
    static QString ID;

public:
    explicit FernsehserienDe(FernsehserienDeConfiguration& settings, QObject* parent = nullptr);
    ~FernsehserienDe() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

private:
    FernsehserienDeConfiguration& m_settings;
    ScraperMeta m_meta;
    FernsehserienDeApi m_api;
};


/// \brief   FernsehserienDe show search job for testing purposes.
/// \details This class can be used as a placeholder for new scrapers that did
///          not yet implement certain features.
class FernsehserienDeShowSearchJob : public ShowSearchJob
{
    Q_OBJECT

public:
    explicit FernsehserienDeShowSearchJob(FernsehserienDeApi& api,
        ShowSearchJob::Config _config,
        QObject* parent = nullptr);
    ~FernsehserienDeShowSearchJob() override = default;
    void doStart() override;

private slots:
    void onSearchPageLoaded();

private:
    QVector<ShowSearchJob::Result> parseSearch(const QString& html);
    ShowSearchJob::Result parseResultFromEpisodePage(const QUrl& url, const QString& html);

private:
    FernsehserienDeApi& m_api;
    int m_redirectionCount{0};
};

/// \brief   FernsehserienDe show scrape job for testing purposes.
/// \details This class can be used as a placeholder for new scrapers that did
///          not yet implement certain features.
class FernsehserienDeShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    FernsehserienDeShowScrapeJob(FernsehserienDeApi& api, Config _config, QObject* parent = nullptr);
    ~FernsehserienDeShowScrapeJob() override = default;
    void doStart() override;

private:
    void parseTvShow(const QString& html);

private:
    FernsehserienDeApi& m_api;
};

/// \brief   FernsehserienDe season scrape job for testing purposes.
/// \details This class can be used as a placeholder for new scrapers that did
///          not yet implement certain features.
class FernsehserienDeSeasonScrapeJob : public SeasonScrapeJob
{
    Q_OBJECT

public:
    FernsehserienDeSeasonScrapeJob(FernsehserienDeApi& api, Config _config, QObject* parent = nullptr);
    ~FernsehserienDeSeasonScrapeJob() = default;
    void doStart() override;

private slots:
    void onSeasonPageLoaded();

private:
    /// Used for download list of episodes.
    struct EpisodeEntry
    {
        SeasonNumber seasonNumber;
        EpisodeNumber episodeNumber;
        QString id;
    };

private:
    void loadNextEpisode();
    void parseSeasonPageAndStartLoading(const QString& html);

private:
    FernsehserienDeApi& m_api;
    QVector<EpisodeEntry> m_episodeIds;
    int m_redirectionCount{0};
};

/// \brief   FernsehserienDe episode scrape job for testing purposes.
/// \details This class can be used as a placeholder for new scrapers that did
///          not yet implement certain features.
class FernsehserienDeEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    FernsehserienDeEpisodeScrapeJob(FernsehserienDeApi& api, Config _config, QObject* parent = nullptr);
    ~FernsehserienDeEpisodeScrapeJob() = default;
    void doStart() override;

private slots:
    void onSeasonPageLoaded();

private:
    void loadSeason();
    void loadEpisode(const QString& episodeId);

    void parseEpisode(const QString& html);
    void parseAndLoadEpisodeIdFromSeason(const QString& html);

private:
    FernsehserienDeApi& m_api;
    int m_redirectionCount{0};
};

} // namespace scraper
} // namespace mediaelch
