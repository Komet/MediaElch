#pragma once

#include "scrapers/tv_show/SeasonScrapeJob.h"

namespace mediaelch {
namespace scraper {

class OmdbApi;

class OmdbTvSeasonScrapeJob : public SeasonScrapeJob
{
    Q_OBJECT

public:
    OmdbTvSeasonScrapeJob(OmdbApi& api, Config _config, QObject* parent = nullptr);
    ~OmdbTvSeasonScrapeJob() override = default;
    void doStart() override;

private:
    void loadSeason(SeasonNumber season);
    void onSeasonLoaded();

private:
    OmdbApi& m_api;
    QSet<SeasonNumber> m_seasonsLeft;
};

} // namespace scraper
} // namespace mediaelch
