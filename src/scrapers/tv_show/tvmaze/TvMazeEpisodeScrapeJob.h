#pragma once

#include "data/TvMazeId.h"
#include "scrapers/tv_show/EpisodeScrapeJob.h"

namespace mediaelch {
namespace scraper {

class TvMazeApi;

class TvMazeEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    TvMazeEpisodeScrapeJob(TvMazeApi& api, Config config, QObject* parent = nullptr);
    ~TvMazeEpisodeScrapeJob() override = default;
    void doStart() override;

private:
    void loadEpisode(const TvMazeId& episodeId);
    void loadAllEpisodes(const TvMazeId& showId);

private:
    TvMazeApi& m_api;
};


} // namespace scraper
} // namespace mediaelch
