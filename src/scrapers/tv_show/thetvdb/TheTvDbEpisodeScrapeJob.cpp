#include "TheTvDbEpisodeScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"
#include "scrapers/tv_show/thetvdb/TheTvDbEpisodeParser.h"
#include "settings/Settings.h"

#include <QObject>
#include <QString>
#include <utility>

namespace mediaelch {
namespace scraper {

TheTvDbEpisodeScrapeJob::TheTvDbEpisodeScrapeJob(TheTvDbApi& api, Config config, QObject* parent) :
    EpisodeScrapeJob(config, parent), m_api{api}
{
    setParent(parent);
}

void TheTvDbEpisodeScrapeJob::doStart()
{
    if (config().identifier.hasEpisodeIdentifier()) {
        loadEpisode(TvDbId(config().identifier.episodeIdentifier));
    } else {
        loadSeason();
    }
}

void TheTvDbEpisodeScrapeJob::loadSeason()
{
    qCDebug(generic) << "[TheTvDbEpisodeScrapeJob] Have to load season first for show:"
                     << config().identifier.showIdentifier;

    // The episode parser requires season/episode to be set when
    // calling parseIdFromSeason()
    episode().setSeason(config().identifier.seasonNumber);
    episode().setEpisode(config().identifier.episodeNumber);

    m_api.loadSeason(config().locale,
        TvDbId(config().identifier.showIdentifier),
        config().identifier.seasonNumber,
        config().identifier.seasonOrder,
        [this](QJsonDocument json, ScraperError error) {
            if (!error.hasError()) {
                // TODO: It is possible that results are paginated.
                TheTvDbEpisodeParser parser(episode(), config().identifier.seasonOrder);
                parser.parseIdFromSeason(json);
                loadEpisode(episode().tvdbId());
            } else {
                setScraperError(error);
                emitFinished();
            }
        });
}

void TheTvDbEpisodeScrapeJob::loadEpisode(const TvDbId& episodeId)
{
    if (!episodeId.isValid()) {
        qCWarning(generic) << "[TheTvDbEpisodeScrapeJob] Invalid TheTvDb ID, cannot scrape episode!";
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("TheTvDb ID is invalid! Cannot load requested episode.");
        setScraperError(error);
        emitFinished();
        return;
    }

    qCDebug(generic) << "[TheTvDbEpisodeScrapeJob] Loading episode with id:" << episodeId;
    m_api.loadEpisode(config().locale, episodeId, [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            TheTvDbEpisodeParser parser(episode(), config().identifier.seasonOrder);
            parser.parseInfos(json);
        } else {
            setScraperError(error);
        }
        emitFinished();
    });
}

} // namespace scraper
} // namespace mediaelch
