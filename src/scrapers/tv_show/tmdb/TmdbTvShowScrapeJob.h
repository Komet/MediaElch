#pragma once

#include "data/TmdbId.h"
#include "scrapers/tv_show/ShowScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowParser.h"

namespace mediaelch {
namespace scraper {

class TmdbApi;

class TmdbTvShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    TmdbTvShowScrapeJob(TmdbApi& api, Config _config, QObject* parent = nullptr);
    ~TmdbTvShowScrapeJob() override = default;
    void doStart() override;

private:
    void loadTvShow();
    /// \brief Request show infos in English when the scrape locale is not English.
    void loadEnglishTitle();

private:
    TmdbApi& m_api;
    TmdbTvShowParser m_parser;
    TmdbId m_id;
};

} // namespace scraper
} // namespace mediaelch
