#include "TheTvDbEpisodeScrapeJob.h"

#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"
#include "scrapers/tv_show/thetvdb/TheTvDbEpisodeParser.h"
#include "settings/Settings.h"
#include "tv_shows/TvShowEpisode.h"

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

void TheTvDbEpisodeScrapeJob::execute()
{
    if (config().identifier.hasEpisodeIdentifier()) {
        loadEpisode(TvDbId(config().identifier.episodeIdentifier));
    } else {
        loadSeason();
    }
}

void TheTvDbEpisodeScrapeJob::loadSeason()
{
    qDebug() << "[TheTvDbEpisodeScrapeJob] Have to load season first for show:" << config().identifier.showIdentifier;

    // The episode parser requires season/episode to be set when
    // calling parseIdFromSeason()
    episode().setSeason(config().identifier.seasonNumber);
    episode().setEpisode(config().identifier.episodeNumber);

    m_api.loadSeason(config().locale,
        TvDbId(config().identifier.showIdentifier),
        config().identifier.seasonNumber,
        config().identifier.seasonOrder,
        [this](QString json) {
            // TODO: It is possible that results are paginated.
            TheTvDbEpisodeParser parser(episode(), config().identifier.seasonOrder);
            parser.parseIdFromSeason(json);
            loadEpisode(episode().tvdbId());
        });
}

void TheTvDbEpisodeScrapeJob::loadEpisode(const TvDbId& episodeId)
{
    if (!episodeId.isValid()) {
        qWarning() << "[TheTvDbEpisodeScrapeJob] Invalid TheTvDb ID, cannot scrape episode!";
        m_error.error = ScraperError::ErrorType::ConfigError;
        m_error.message = tr("TheTvDb ID is invalid! Cannot load requested episode.");
        QTimer::singleShot(0, [this]() { emit sigFinished(this); });
        return;
    }
    qDebug() << "[TheTvDbEpisodeScrapeJob] Loading episode with id:" << episodeId;
    m_api.loadEpisode(config().locale, episodeId, [this](QString json) {
        TheTvDbEpisodeParser parser(episode(), config().identifier.seasonOrder);
        parser.parseInfos(json);
        emit sigFinished(this);
    });
}

} // namespace scraper
} // namespace mediaelch
