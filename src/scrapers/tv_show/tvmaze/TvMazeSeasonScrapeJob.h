
#pragma once

#include "data/TvMazeId.h"
#include "scrapers/tv_show/SeasonScrapeJob.h"

namespace mediaelch {
namespace scraper {

class TvMazeApi;

class TvMazeSeasonScrapeJob : public SeasonScrapeJob
{
    Q_OBJECT

public:
    TvMazeSeasonScrapeJob(TvMazeApi& api, Config _config, QObject* parent = nullptr);
    ~TvMazeSeasonScrapeJob() override = default;

public:
    void doStart() override;

private:
    TvMazeApi& m_api;
    TvMazeId m_showId;
};

} // namespace scraper
} // namespace mediaelch
