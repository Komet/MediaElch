#include "scrapers/tv_show/tvmaze/TvMazeSeasonScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"
#include "log/Log.h"
#include "scrapers/tv_show/tvmaze/TvMazeApi.h"
#include "scrapers/tv_show/tvmaze/TvMazeEpisodeParser.h"

#include <QJsonArray>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

TvMazeSeasonScrapeJob::TvMazeSeasonScrapeJob(TvMazeApi& api, SeasonScrapeJob::Config _config, QObject* parent) :
    SeasonScrapeJob(_config, parent), m_api{api}, m_showId{TvMazeId(config().showIdentifier.str())}
{
}

void TvMazeSeasonScrapeJob::doStart()
{
    if (!m_showId.isValid()) {
        qCWarning(generic) << "[TmdbTv] Provided Tmdb id is invalid:" << config().showIdentifier;
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("Show is missing a TMDB id");
        setScraperError(error);
        emitFinished();
        return;
    }

    // Simply load all episodes for the show.
    // TVmaze does not have an API for scraping a single season.
    // Furthermore this makes it possible to use the cache and avoid reaching
    // their rate limit.

    m_api.loadAllEpisodes(m_showId, [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }

        QJsonArray episodes = json.array();
        for (const QJsonValue& val : asConst(episodes)) {
            QJsonObject episodeObj = val.toObject();

            const int seasonNumber = episodeObj["season"].toInt(-2);
            const SeasonNumber season(seasonNumber);

            if (seasonNumber > -2 && (config().shouldLoadAllSeasons() || config().seasons.contains(season))) {
                auto* episode = new TvShowEpisode({}, this);
                TvMazeEpisodeParser::parseEpisode(*episode, episodeObj);
                m_episodes[{season, episode->episodeNumber()}] = episode;
            }
        }

        emitFinished();
    });
}

} // namespace scraper
} // namespace mediaelch
