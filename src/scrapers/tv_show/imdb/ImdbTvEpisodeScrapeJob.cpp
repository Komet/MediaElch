#include "scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h"

#include "scrapers/tv_show/imdb/ImdbTvApi.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"
#include "tv_shows/TvShowEpisode.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

ImdbTvEpisodeScrapeJob::ImdbTvEpisodeScrapeJob(ImdbTvApi& api, EpisodeScrapeJob::Config _config, QObject* parent) :
    EpisodeScrapeJob(_config, parent), m_api{api}
{
}

void ImdbTvEpisodeScrapeJob::execute()
{
    if (config().identifier.hasEpisodeIdentifier()) {
        loadEpisode(ImdbId(config().identifier.episodeIdentifier));
    } else {
        loadSeason();
    }
}

void ImdbTvEpisodeScrapeJob::loadSeason()
{
    qDebug() << "[ImdbTvEpisodeScrapeJob] Have to load season first.";

    ImdbId showId(config().identifier.showIdentifier);

    if (!showId.isValid()) {
        qWarning() << "[ImdbTvEpisodeScrapeJob] Invalid IMDb ID for TV show, cannot scrape episode!";
        m_error.error = ScraperLoadError::ErrorType::ConfigError;
        m_error.message = tr("Neither IMDb show ID nor episode ID are valid! Cannot load requested episode.");
        QTimer::singleShot(0, [this]() { emit sigFinished(this); });
        return;
    }

    // The episode parser requires season/episode to be set when
    // calling parseIdFromSeason()
    episode().setSeason(config().identifier.seasonNumber);
    episode().setEpisode(config().identifier.episodeNumber);

    m_api.loadSeason(config().locale, showId, config().identifier.seasonNumber, [this, showId](QString html) {
        ImdbTvEpisodeParser::parseIdFromSeason(episode(), html);
        if (!episode().imdbId().isValid()) {
            qWarning() << "[ImdbTvEpisodeScrapeJob] Could not parse IMDb ID for episode from season page!";
            m_error.error = ScraperLoadError::ErrorType::ConfigError;
            m_error.message = tr("IMDb ID could not be loaded from season page! Cannot load requested episode.");
            emit sigFinished(this);
        } else {
            loadEpisode(episode().imdbId());
        }
    });
}

void ImdbTvEpisodeScrapeJob::loadEpisode(const ImdbId& episodeId)
{
    if (!episodeId.isValid()) {
        qWarning() << "[ImdbTvEpisodeScrapeJob] Invalid IMDb ID, cannot scrape episode!";
        m_error.error = ScraperLoadError::ErrorType::ConfigError;
        m_error.message = tr("IMDb ID is invalid! Cannot load requested episode.");
        QTimer::singleShot(0, [this]() { emit sigFinished(this); });
        return;
    }

    qInfo() << "[ImdbTvEpisodeScrapeJob] Loading episode with IMDb ID" << episodeId.toString();
    m_api.loadEpisode(config().locale, episodeId, [this](QString html) {
        ImdbTvEpisodeParser::parseInfos(episode(), html);
        emit sigFinished(this);
    });
}

} // namespace scraper
} // namespace mediaelch
