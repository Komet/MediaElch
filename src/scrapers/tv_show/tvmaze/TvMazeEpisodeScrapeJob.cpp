#include "TvMazeEpisodeScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"
#include "log/Log.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/tvmaze/TvMazeApi.h"
#include "scrapers/tv_show/tvmaze/TvMazeEpisodeParser.h"
#include "settings/Settings.h"

#include <QObject>
#include <QString>
#include <utility>

namespace mediaelch {
namespace scraper {

TvMazeEpisodeScrapeJob::TvMazeEpisodeScrapeJob(TvMazeApi& api, Config config, QObject* parent) :
    EpisodeScrapeJob(config, parent), m_api{api}
{
}

void TvMazeEpisodeScrapeJob::doStart()
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
        qCWarning(generic) << "[TvMazeEpisodeScrapeJob] Invalid TVmaze ID for TV show, cannot scrape episode!";
        ScraperError configError;
        configError.error = ScraperError::Type::ConfigError;
        configError.message = tr("TVmaze show ID are valid! Cannot load requested episode.");
        setScraperError(configError);
        emitFinished();
        return;
    }

    // The episode parser requires season/episode to be set when
    // calling parseEpisodeFromOverview()
    episode().setSeason(config().identifier.seasonNumber);
    episode().setEpisode(config().identifier.episodeNumber);

    m_api.loadAllEpisodes(showId, [this, showId](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
        } else {
            TvMazeEpisodeParser::parseEpisodeFromOverview(episode(), json);
            // The "all episodes" page only contains basic details.
            // To get all details we need, load details based on the episode's ID.
            if (episode().tvmazeId().isValid()) {
                const TvMazeId id = episode().tvmazeId();
                episode().clear(); // avoid re-loading ratings, etc.
                loadEpisode(id);
            } else {
                emitFinished();
            }
        }
    });
}

void TvMazeEpisodeScrapeJob::loadEpisode(const TvMazeId& episodeId)
{
    if (!episodeId.isValid()) {
        qCWarning(generic) << "[TvMazeEpisodeScrapeJob] Invalid TVmaze ID, cannot scrape episode!";
        ScraperError configError;
        configError.error = ScraperError::Type::ConfigError;
        configError.message = tr("TVmaze ID is invalid! Cannot load requested episode.");
        setScraperError(configError);
        emitFinished();
        return;
    }

    qCInfo(generic) << "[TvMazeEpisodeScrapeJob] Loading episode with TVmaze ID" << episodeId.toString();
    m_api.loadEpisode(episodeId, [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
        } else {
            TvMazeEpisodeParser::parseEpisode(episode(), json);
        }
        emitFinished();
    });
}


} // namespace scraper
} // namespace mediaelch
