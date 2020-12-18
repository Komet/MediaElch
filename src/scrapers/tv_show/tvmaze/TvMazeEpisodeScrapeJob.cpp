#include "TvMazeEpisodeScrapeJob.h"

#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/tvmaze/TvMazeApi.h"
#include "scrapers/tv_show/tvmaze/TvMazeEpisodeParser.h"
#include "settings/Settings.h"
#include "tv_shows/TvShowEpisode.h"

#include <QObject>
#include <QString>
#include <utility>

namespace mediaelch {
namespace scraper {

TvMazeEpisodeScrapeJob::TvMazeEpisodeScrapeJob(TvMazeApi& api, Config config, QObject* parent) :
    EpisodeScrapeJob(config, parent), m_api{api}
{
}

void TvMazeEpisodeScrapeJob::execute()
{
    if (config().identifier.hasEpisodeIdentifier()) {
        loadEpisode(TvMazeId(config().identifier.episodeIdentifier));

    } else {
        loadAllEpisodes(TvMazeId(config().identifier.showIdentifier));
    }
}


void TvMazeEpisodeScrapeJob::loadAllEpisodes(const TvMazeId& showId)
{
    if (!showId.isValid()) {
        qWarning() << "[TvMazeEpisodeScrapeJob] Invalid TVmaze ID for TV show, cannot scrape episode!";
        m_error.error = ScraperError::Type::ConfigError;
        m_error.message = tr("TVmaze show ID are valid! Cannot load requested episode.");
        emit sigFinished(this);
        return;
    }

    // The episode parser requires season/episode to be set when
    // calling parseEpisodeFromOverview()
    episode().setSeason(config().identifier.seasonNumber);
    episode().setEpisode(config().identifier.episodeNumber);

    m_api.loadAllEpisodes(showId, [this, showId](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
        } else {
            TvMazeEpisodeParser::parseEpisodeFromOverview(episode(), json);
        }
        emit sigFinished(this);
    });
}

void TvMazeEpisodeScrapeJob::loadEpisode(const TvMazeId& episodeId)
{
    if (!episodeId.isValid()) {
        qWarning() << "[TvMazeEpisodeScrapeJob] Invalid TVmaze ID, cannot scrape episode!";
        m_error.error = ScraperError::Type::ConfigError;
        m_error.message = tr("TVmaze ID is invalid! Cannot load requested episode.");
        emit sigFinished(this);
        return;
    }

    qInfo() << "[TvMazeEpisodeScrapeJob] Loading episode with TVmaze ID" << episodeId.toString();
    m_api.loadEpisode(episodeId, [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
        } else {
            TvMazeEpisodeParser::parseEpisode(episode(), json);
        }
        emit sigFinished(this);
    });
}


} // namespace scraper
} // namespace mediaelch
