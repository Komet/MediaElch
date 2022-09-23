#pragma once

#include "data/TvDbId.h"
#include "scrapers/tv_show/EpisodeScrapeJob.h"

namespace mediaelch {
namespace scraper {

class TheTvDbApi;

class TheTvDbEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    TheTvDbEpisodeScrapeJob(TheTvDbApi& api, Config config, QObject* parent = nullptr);
    ~TheTvDbEpisodeScrapeJob() override = default;
    void doStart() override;

private:
    void loadSeason();
    void loadEpisode(const TvDbId& episodeId);

private:
    TheTvDbApi& m_api;
    QSet<ShowScraperInfo> m_loaded;
};


} // namespace scraper
} // namespace mediaelch
