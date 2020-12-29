#include "scrapers/tv_show/tvmaze/TvMazeSeasonScrapeJob.h"

#include "scrapers/tv_show/tvmaze/TvMazeApi.h"
#include "scrapers/tv_show/tvmaze/TvMazeEpisodeParser.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

TvMazeSeasonScrapeJob::TvMazeSeasonScrapeJob(TvMazeApi& api, SeasonScrapeJob::Config _config, QObject* parent) :
    SeasonScrapeJob(_config, parent), m_api{api}, m_showId{TvMazeId(config().showIdentifier.str())}
{
}

void TvMazeSeasonScrapeJob::execute()
{
    if (!m_showId.isValid()) {
        qWarning() << "[TmdbTv] Provided Tmdb id is invalid:" << config().showIdentifier;
        m_error.error = ScraperError::Type::ConfigError;
        m_error.message = tr("Show is missing a TMDb id");
        emit sigFinished(this);
        return;
    }

    // Simply load all episodes for the show.
    // TVmaze does not have an API for scraping a single season.
    // Furthermore this makes it possible to use the cache and avoid reaching
    // their rate limit.

    m_api.loadAllEpisodes(m_showId, [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
            emit sigFinished(this);
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

        emit sigFinished(this);
    });
}

} // namespace scraper
} // namespace mediaelch
