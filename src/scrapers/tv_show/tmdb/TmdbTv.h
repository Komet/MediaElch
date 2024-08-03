#pragma once

#include "scrapers/tmdb/TmdbApi.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/tmdb/TmdbTvConfiguration.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace scraper {

class TmdbTv : public TvScraper
{
    Q_OBJECT

public:
    static QString ID;

public:
    explicit TmdbTv(TmdbTvConfiguration& settings, QObject* parent = nullptr);
    ~TmdbTv() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

private:
    TmdbTvConfiguration& m_settings;
    ScraperMeta m_meta;
    TmdbApi m_api;
};

} // namespace scraper
} // namespace mediaelch
