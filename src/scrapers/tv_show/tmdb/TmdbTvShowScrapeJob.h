#pragma once

#include "data/TmdbId.h"
#include "scrapers/tv_show/ShowScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowParser.h"

namespace mediaelch {
namespace scraper {

class TmdbTvApi;

class TmdbTvShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    TmdbTvShowScrapeJob(TmdbTvApi& api, Config _config, QObject* parent = nullptr);
    ~TmdbTvShowScrapeJob() override = default;
    void execute() override;

private:
    void loadTvShow();

private:
    TmdbTvApi& m_api;
    TmdbTvShowParser m_parser;
    TmdbId m_id;
};

} // namespace scraper
} // namespace mediaelch
