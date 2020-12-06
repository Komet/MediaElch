#pragma once

#include "globals/Meta.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/tmdb/TmdbTvApi.h"

namespace mediaelch {
namespace scraper {

class TmdbTv : public TvScraper
{
    Q_OBJECT

public:
    static QString ID;

public:
    explicit TmdbTv(QObject* parent = nullptr);
    ~TmdbTv() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

private:
    ScraperMeta m_meta;
    TmdbTvApi m_api;
    bool m_isInitialized = false;
};

} // namespace scraper
} // namespace mediaelch
