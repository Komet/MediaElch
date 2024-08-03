#pragma once

#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/tvmaze/TvMazeApi.h"
#include "scrapers/tv_show/tvmaze/TvMazeConfiguration.h"
#include "utils/Meta.h"

namespace mediaelch {
namespace scraper {

class TvMaze : public TvScraper
{
    Q_OBJECT

public:
    static QString ID;

public:
    explicit TvMaze(TvMazeConfiguration& settings, QObject* parent = nullptr);
    ~TvMaze() override = default;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ShowSearchJob* search(ShowSearchJob::Config config) override;
    ShowScrapeJob* loadShow(ShowScrapeJob::Config config) override;
    SeasonScrapeJob* loadSeasons(SeasonScrapeJob::Config config) override;
    EpisodeScrapeJob* loadEpisode(EpisodeScrapeJob::Config config) override;

private:
    TvMazeConfiguration& m_settings;
    ScraperMeta m_meta;
    TvMazeApi m_api;
};

} // namespace scraper
} // namespace mediaelch
