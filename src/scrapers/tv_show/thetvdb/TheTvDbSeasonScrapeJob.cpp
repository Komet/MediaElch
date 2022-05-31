#include "TheTvDbSeasonScrapeJob.h"

#include "log/Log.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"
#include "scrapers/tv_show/thetvdb/TheTvDbSeasonScrapeJob.h"
#include "tv_shows/TvShow.h"

#include <QObject>
#include <utility>

namespace mediaelch {
namespace scraper {

TheTvDbSeasonScrapeJob::TheTvDbSeasonScrapeJob(TheTvDbApi& api, Config _config, QObject* parent) :
    SeasonScrapeJob(std::move(_config), parent), m_api{api}, m_showId{config().showIdentifier.str()}
{
}

void TheTvDbSeasonScrapeJob::doStart()
{
    if (!m_showId.isValid()) {
        qCWarning(generic) << "[TheTvDb] Provided TheTvDb id is invalid:" << config().showIdentifier;
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("Show is missing a TheTvDb id");
        setScraperError(error);
        QTimer::singleShot(0, this, [this]() { emitFinished(); });
        return;
    }
    loadEpisodePage(TheTvDbApi::ApiPage{1});
}

void TheTvDbSeasonScrapeJob::loadEpisodePage(TheTvDbApi::ApiPage page)
{
    const auto callback = [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            const auto onEpisode = [this](TvShowEpisode* episode) { storeEpisode(episode); };
            // Pass `this` so that newly generated episodes belong to this instance.
            const auto next =
                mediaelch::scraper::TheTvDbEpisodesParser::parseEpisodes(json, config().seasonOrder, this, onEpisode);
            if (next.hasNextPage()) {
                loadEpisodePage(next.next);
            } else {
                emitFinished();
            }
        } else {
            setScraperError(error);
            emitFinished();
        }
    };
    if (config().shouldLoadAllSeasons()) {
        m_api.loadAllSeasonsPage(config().locale, m_showId, config().seasonOrder, page, callback);
    } else {
        m_api.loadSeasonsPage(config().locale, m_showId, config().seasons, config().seasonOrder, page, callback);
    }
}

void TheTvDbSeasonScrapeJob::storeEpisode(TvShowEpisode* episode)
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
