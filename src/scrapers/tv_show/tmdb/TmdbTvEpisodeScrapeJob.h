#pragma once

#include "scrapers/tv_show/EpisodeScrapeJob.h"

namespace mediaelch {
namespace scraper {

class TmdbTvApi;

class TmdbTvEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    TmdbTvEpisodeScrapeJob(TmdbTvApi& api, Config _config, QObject* parent = nullptr);
    ~TmdbTvEpisodeScrapeJob() = default;
    void execute() override;

private:
    TmdbTvApi& m_api;
};

} // namespace scraper
} // namespace mediaelch
