#pragma once

#include "scrapers/tv_show/SeasonScrapeJob.h"

#include <QList>

namespace mediaelch {
namespace scraper {

class ImdbApi;

class ImdbTvSeasonScrapeJob : public SeasonScrapeJob
{
    Q_OBJECT

public:
    ImdbTvSeasonScrapeJob(ImdbApi& api, Config _config, QObject* parent = nullptr);
    ~ImdbTvSeasonScrapeJob() = default;
    void doStart() override;

private:
    void loadEpisodes();
    void parseAndStoreEpisodes(const QString& json);
    void storeEpisode(TvShowEpisode* episode);

private:
    ImdbApi& m_api;
    ImdbId m_showId;
};

} // namespace scraper
} // namespace mediaelch
