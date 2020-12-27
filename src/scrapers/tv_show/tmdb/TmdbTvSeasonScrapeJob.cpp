#include "scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.h"

#include "scrapers/tmdb/TmdbApi.h"
#include "scrapers/tv_show/tmdb/TmdbTvSeasonParser.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>
#include <QTimer>

namespace mediaelch {
namespace scraper {

TmdbTvSeasonScrapeJob::TmdbTvSeasonScrapeJob(TmdbApi& api, SeasonScrapeJob::Config _config, QObject* parent) :
    SeasonScrapeJob(_config, parent), m_api{api}, m_showId{TmdbId(config().showIdentifier.str())}
{
}

void TmdbTvSeasonScrapeJob::execute()
{
    if (!m_showId.isValid()) {
        qWarning() << "[TmdbTv] Provided Tmdb id is invalid:" << config().showIdentifier;
        m_error.error = ScraperError::Type::ConfigError;
        m_error.message = tr("Show is missing a TMDb id");
        emit sigFinished(this);
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
        emit sigFinished(this);
        return;
    }

    const SeasonNumber nextSeason = seasons.takeFirst();
    const TmdbApi::ApiCallback callback = [this, seasons](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
            emit sigFinished(this);
            return;
        }
        const auto onEpisode = [this](TvShowEpisode* episode) { storeEpisode(episode); };
        // Pass `this` so that newly generated episodes belong to this instance.
        TmdbTvSeasonParser::parseEpisodes(m_api, json, this, onEpisode);
        loadSeasons(seasons);
    };

    m_api.loadSeason(config().locale, m_showId, nextSeason, config().seasonOrder, callback);
}

void TmdbTvSeasonScrapeJob::loadAllSeasons()
{
    m_api.loadMinimalInfos(config().locale, m_showId, [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
            emit sigFinished(this);
            return;
        }
        QSet<SeasonNumber> seasons;
        const QJsonArray seasonArray = json.object()["seasons"].toArray();
        for (const auto& season : seasonArray) {
            int number = season.toObject()["season_number"].toInt(-1);
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
        m_episodes[{season, episode->episodeNumber()}] = episode;
    } else {
        // Only store episodes that are actually requested.
        episode->deleteLater();
    }
}

} // namespace scraper
} // namespace mediaelch
