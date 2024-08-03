#pragma once

#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"
#include "scrapers/tv_show/thetvdb/TheTvDbConfiguration.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace scraper {

class TheTvDb : public TvScraper
{
    Q_OBJECT

public:
    static QString ID;

public:
    explicit TheTvDb(TheTvDbConfiguration& settings, QObject* parent = nullptr);
    ~TheTvDb() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

private:
    TheTvDbConfiguration& m_settings;
    ScraperMeta m_meta;
    TheTvDbApi m_api;
    bool m_isInitialized = false;
};

} // namespace scraper
} // namespace mediaelch
