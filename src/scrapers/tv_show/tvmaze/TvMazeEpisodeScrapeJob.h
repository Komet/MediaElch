#pragma once

#include "scrapers/tv_show/EpisodeScrapeJob.h"
#include "tv_shows/TvMazeId.h"

namespace mediaelch {
namespace scraper {

class TvMazeApi;

class TvMazeEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    TvMazeEpisodeScrapeJob(TvMazeApi& api, Config config, QObject* parent = nullptr);
    ~TvMazeEpisodeScrapeJob() override = default;
    void execute() override;

private:
    void loadEpisode(const TvMazeId& episodeId);
    void loadAllEpisodes(const TvMazeId& showId);

private:
    TvMazeApi& m_api;
};


} // namespace scraper
} // namespace mediaelch
