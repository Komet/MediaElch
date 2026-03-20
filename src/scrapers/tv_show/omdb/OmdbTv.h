#pragma once

#include "scrapers/omdb/OmdbApi.h"
#include "scrapers/tv_show/TvScraper.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace scraper {

class OmdbTvConfiguration;

class OmdbTv : public TvScraper
{
    Q_OBJECT

public:
    static constexpr const char* ID = "omdbtv";

public:
    explicit OmdbTv(OmdbTvConfiguration& settings, QObject* parent = nullptr);
    ~OmdbTv() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

private:
    OmdbTvConfiguration& m_settings;
    ScraperMeta m_meta;
    OmdbApi m_api;
};

} // namespace scraper
} // namespace mediaelch
