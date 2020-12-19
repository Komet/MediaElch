
#pragma once

#include "scrapers/tv_show/SeasonScrapeJob.h"
#include "scrapers/tv_show/thetvdb/TheTvDbEpisodesParser.h"

#include <QMutex>
#include <QSet>

namespace mediaelch {
namespace scraper {

class TheTvDbApi;

class TheTvDbSeasonScrapeJob : public SeasonScrapeJob
{
    Q_OBJECT

public:
    TheTvDbSeasonScrapeJob(TheTvDbApi& api, Config _config, QObject* parent = nullptr);
    ~TheTvDbSeasonScrapeJob() override = default;
    void execute() override;

private:
    void loadEpisodePage(TheTvDbApi::ApiPage page);
    void storeEpisode(TvShowEpisode* episode);

private:
    TheTvDbApi& m_api;
    TvDbId m_showId;
    TheTvDbEpisodesParser m_parser;
};

} // namespace scraper
} // namespace mediaelch
