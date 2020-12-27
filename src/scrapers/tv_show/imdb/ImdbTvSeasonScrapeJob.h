#pragma once

#include "scrapers/tv_show/SeasonScrapeJob.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"

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
    void execute() override;

private:
    /// \brief Loads the given episodes in a sequential way
    /// \todo Load in parallel.
    void loadEpisodes(QMap<SeasonNumber, QMap<EpisodeNumber, ImdbId>> episodeIds);
    /// \brief Gathers all episode IDs for the given seasons by loading each
    ///        season page and then calls laodEpisodes().
    void gatherAndLoadEpisodes(QList<SeasonNumber> seasonsToLoad,
        QMap<SeasonNumber, QMap<EpisodeNumber, ImdbId>> episodeIds);
    void loadAllSeasons();
    /// \brief Store the given episode in the internal season-episode map.
    void storeEpisode(TvShowEpisode* episode);

private:
    ImdbApi& m_api;
    ImdbId m_showId;
};

} // namespace scraper
} // namespace mediaelch
