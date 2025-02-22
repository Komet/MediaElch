#include "scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"
#include "log/Log.h"
#include "scrapers/tmdb/TmdbApi.h"
#include "scrapers/tv_show/tmdb/TmdbTvSeasonParser.h"

#include <QJsonArray>
#include <QTimer>

namespace mediaelch {
namespace scraper {

TmdbTvSeasonScrapeJob::TmdbTvSeasonScrapeJob(TmdbApi& api, SeasonScrapeJob::Config _config, QObject* parent) :
    SeasonScrapeJob(_config, parent), m_api{api}, m_showId{TmdbId(config().showIdentifier.str())}
{
}

void TmdbTvSeasonScrapeJob::doStart()
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

    if (config().shouldLoadAllSeasons()) {
        loadAllSeasons();

    } else {
        loadSeasons(config().seasons.values());
    }
}

void TmdbTvSeasonScrapeJob::loadSeasons(QList<SeasonNumber> seasons)
{
    if (seasons.isEmpty()) {
        emitFinished();
        return;
    }

    const SeasonNumber nextSeason = seasons.takeFirst();
    const TmdbApi::ApiCallback callback = [this, season = nextSeason, seasons](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            // don't abort here, as we want to load _all_ seasons before returning an error.
            // TODO: Collect all season error instead of overriding the previous one.

        } else {
            m_actors[season] = TmdbTvSeasonParser::parseSeasonActors(m_api, json);
            const auto onEpisode = [this](TvShowEpisode* episode) { storeEpisode(episode); };
            // Pass `this` so that newly generated episodes belong to this instance.
            TmdbTvSeasonParser::parseEpisodes(m_api, json, this, onEpisode);
        }
        loadSeasons(seasons);
    };

    m_api.loadSeason(config().locale, m_showId, nextSeason, config().seasonOrder, callback);
}

void TmdbTvSeasonScrapeJob::loadAllSeasons()
{
    m_api.loadMinimalDetails(config().locale, m_showId, [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }
        QSet<SeasonNumber> seasons;
        QJsonObject rootObject = json.object();
        const QJsonArray seasonArray = rootObject["seasons"].toArray();
        for (const auto& season : seasonArray) {
            QJsonObject seasonObj = season.toObject();
            int number = seasonObj["season_number"].toInt(-1);
            if (number >= 0) {
                seasons.insert(SeasonNumber(number));
            }
        }
        loadSeasons(seasons.values());
    });
}

void TmdbTvSeasonScrapeJob::storeEpisode(TvShowEpisode* episode)
{
    const SeasonNumber season = episode->seasonNumber();
    if (config().shouldLoadAllSeasons() || config().seasons.contains(season)) {
        // Add all actors of the season to each episode.
        MediaElch_Debug_Ensures(m_actors.contains(season));
        const auto& actors = m_actors[season];
        for (const Actor& actor : actors) {
            episode->addActor(actor);
        }
        m_episodes[{season, episode->episodeNumber()}] = episode;
    } else {
        // Only store episodes that are actually requested.
        episode->deleteLater();
    }
}

} // namespace scraper
} // namespace mediaelch
