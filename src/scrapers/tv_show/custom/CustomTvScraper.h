#pragma once

#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/custom/CustomTvScraperConfig.h"

namespace mediaelch {
namespace scraper {

/// \brief   The custom TV scraper has a hard-coded list of scrapers that it supports.
/// \details The custom TV scraper uses TMDb to load basic details. It then uses the scraped
///          IDs of TheTvDb, etc. to load details from those sites if necessary.
class CustomTvScraper : public TvScraper
{
    Q_OBJECT

public:
    static QString ID;

public:
    static QVector<QString> supportedScraperIds();

public:
    CustomTvScraper(CustomTvScraperConfig config, QObject* parent = nullptr);
    ~CustomTvScraper() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

    QVector<TvScraper*> supportedScrapers() const;

private:
    void updateScraperDetails(const QSet<ShowScraperInfo>& details);
    void updateScraperDetails(const QSet<EpisodeScraperInfo>& details);

private:
    ScraperMeta m_meta;
    CustomTvScraperConfig m_customConfig;
};


} // namespace scraper
} // namespace mediaelch
