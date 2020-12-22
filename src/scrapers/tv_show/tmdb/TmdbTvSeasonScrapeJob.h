#pragma once

#include "scrapers/tv_show/SeasonScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvEpisodeParser.h"

#include <QList>

namespace mediaelch {
namespace scraper {

class TmdbApi;

class TmdbTvSeasonScrapeJob : public SeasonScrapeJob
{
    Q_OBJECT

public:
    TmdbTvSeasonScrapeJob(TmdbApi& api, Config _config, QObject* parent = nullptr);
    ~TmdbTvSeasonScrapeJob() = default;
    void execute() override;

private:
    void loadSeasons(QList<SeasonNumber> seasons);
    void loadAllSeasons();
    void storeEpisode(TvShowEpisode* episode);

private:
    TmdbApi& m_api;
    TmdbId m_showId;
};

} // namespace scraper
} // namespace mediaelch
