#pragma once

#include "scrapers/tv_show/EpisodeScrapeJob.h"

namespace mediaelch {
namespace scraper {

class TmdbApi;

class TmdbTvEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    TmdbTvEpisodeScrapeJob(TmdbApi& api, Config _config, QObject* parent = nullptr);
    ~TmdbTvEpisodeScrapeJob() = default;
    void execute() override;

private:
    TmdbApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
