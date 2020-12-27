#pragma once

#include "scrapers/tv_show/EpisodeScrapeJob.h"

#include "scrapers/imdb/ImdbApi.h"

namespace mediaelch {
namespace scraper {

class ImdbTvEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    ImdbTvEpisodeScrapeJob(ImdbApi& api, Config _config, QObject* parent = nullptr);
    ~ImdbTvEpisodeScrapeJob() = default;
    void execute() override;

private:
    void loadSeason();
    void loadEpisode(const ImdbId& episodeId);

private:
    ImdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
