#include "scrapers/tv_show/imdb/ImdbTvSeasonScrapeJob.h"

#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/tv_show/imdb/ImdbTvSeasonParser.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>
#include <QTimer>

namespace mediaelch {
namespace scraper {

ImdbTvSeasonScrapeJob::ImdbTvSeasonScrapeJob(ImdbApi& api, SeasonScrapeJob::Config _config, QObject* parent) :
    SeasonScrapeJob(_config, parent), m_api{api}, m_showId{ImdbId(config().showIdentifier.str())}
{
}

void ImdbTvSeasonScrapeJob::execute()
{
    if (!m_showId.isValid()) {
        qWarning() << "[ImdbTv] Provided IMDb id is invalid:" << config().showIdentifier;
        m_error.error = ScraperError::Type::ConfigError;
        m_error.message = tr("Show is missing an IMDb id");
        QTimer::singleShot(0, [this]() { emit sigFinished(this); });
        return;
    }

    if (config().shouldLoadAllSeasons()) {
        loadAllSeasons();

    } else {
        gatherAndLoadEpisodes(config().seasons.values(), {});
    }
}

void ImdbTvSeasonScrapeJob::loadEpisodes(QMap<SeasonNumber, QMap<EpisodeNumber, ImdbId>> episodeIds)
{
    if (episodeIds.isEmpty()) {
        emit sigFinished(this);
        return;
    }

    // Get next episode to load and remove it from episodeIds
    const SeasonNumber nextSeason = episodeIds.keys().first();

    // If there is no episode left in that season then remove it.
    if (episodeIds[nextSeason].isEmpty()) {
        episodeIds.remove(nextSeason);
        loadEpisodes(episodeIds);
        return;
    }

    QMap<EpisodeNumber, ImdbId> episodes = episodeIds[nextSeason];
    const EpisodeNumber nextEpisode = episodes.keys().first();
    const ImdbId nextEpisodeId = episodes[nextEpisode];
    episodeIds[nextSeason].remove(nextEpisode);

    // Create episode: We need to set some details because not everything is available
    // from the single episode page (or can be scraped in a stable manner).
    auto* episode = new TvShowEpisode({}, this);
    episode->setSeason(nextSeason);
    episode->setEpisode(nextEpisode);
    episode->setImdbId(nextEpisodeId);

    qInfo() << "[ImdbTvSeasonScrapeJob] Start loading season" << nextSeason.toInt() << "episode" << nextEpisode.toInt()
            << "of show" << config().showIdentifier.str();

    m_api.loadEpisode(config().locale, nextEpisodeId, [this, episode, episodeIds](QString html, ScraperError error) {
        if (error.hasError()) {
            // only store error but try to load other episodes
            m_error = error;
        } else if (!html.isEmpty()) {
            ImdbTvEpisodeParser::parseInfos(*episode, html);
            storeEpisode(episode);
        }
        loadEpisodes(episodeIds);
    });
}

void ImdbTvSeasonScrapeJob::gatherAndLoadEpisodes(QList<SeasonNumber> seasonsToLoad,
    QMap<SeasonNumber, QMap<EpisodeNumber, ImdbId>> episodeIds)
{
    if (seasonsToLoad.isEmpty()) {
        loadEpisodes(episodeIds);
        return;
    }

    const SeasonNumber nextSeason = seasonsToLoad.takeFirst();
    const ImdbApi::ApiCallback callback = [this, nextSeason, seasonsToLoad, episodeIds](
                                              QString html, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
            emit sigFinished(this);
        } else {
            QMap<EpisodeNumber, ImdbId> episodesForSeason = ImdbTvSeasonParser::parseEpisodeIds(html);
            auto ids = episodeIds;
            ids.insert(nextSeason, episodesForSeason);
            gatherAndLoadEpisodes(seasonsToLoad, ids);
        }
    };

    m_api.loadSeason(config().locale, m_showId, nextSeason, callback);
}

void ImdbTvSeasonScrapeJob::loadAllSeasons()
{
    m_api.loadDefaultEpisodesPage(config().locale, m_showId, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
            emit sigFinished(this);
            return;
        }
        QSet<SeasonNumber> seasons = ImdbTvSeasonParser::parseSeasonNumbersFromEpisodesPage(html);
        gatherAndLoadEpisodes(seasons.values(), {});
    });
}

void ImdbTvSeasonScrapeJob::storeEpisode(TvShowEpisode* episode)
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
