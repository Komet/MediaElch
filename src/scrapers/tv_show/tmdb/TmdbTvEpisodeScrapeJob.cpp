#include "scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.h"

#include "scrapers/tv_show/tmdb/TmdbTvApi.h"
#include "scrapers/tv_show/tmdb/TmdbTvEpisodeParser.h"
#include "tv_shows/TvShowEpisode.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

TmdbTvEpisodeScrapeJob::TmdbTvEpisodeScrapeJob(TmdbTvApi& api, EpisodeScrapeJob::Config _config, QObject* parent) :
    EpisodeScrapeJob(_config, parent), m_api{api}
{
}

void TmdbTvEpisodeScrapeJob::execute()
{
    TmdbId showId(config().identifier.showIdentifier);

    if (!showId.isValid()) {
        qWarning() << "[TmdbTvEpisodeScrapeJob] Invalid TMDb ID for TV show, cannot scrape episode!";
        m_error.error = ScraperLoadError::ErrorType::ConfigError;
        m_error.message = tr("TMDb show ID is invalid! Cannot load requested episode.");
        QTimer::singleShot(0, [this]() { emit sigFinished(this); });
        return;
    }

    qInfo() << "[TmdbTvEpisodeScrapeJob] Have to load season first.";

    m_api.loadEpisode(config().locale,
        showId,
        config().identifier.seasonNumber,
        config().identifier.episodeNumber,
        [this](QJsonDocument json) {
            TmdbTvEpisodeParser::parseInfos(m_api, episode(), json.object());
            emit sigFinished(this);
        });
}

} // namespace scraper
} // namespace mediaelch