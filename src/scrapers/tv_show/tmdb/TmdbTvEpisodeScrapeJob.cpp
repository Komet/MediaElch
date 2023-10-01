#include "scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"
#include "log/Log.h"
#include "scrapers/tmdb/TmdbApi.h"
#include "scrapers/tv_show/tmdb/TmdbTvEpisodeParser.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

TmdbTvEpisodeScrapeJob::TmdbTvEpisodeScrapeJob(TmdbApi& api, EpisodeScrapeJob::Config _config, QObject* parent) :
    EpisodeScrapeJob(_config, parent), m_api{api}
{
}

void TmdbTvEpisodeScrapeJob::doStart()
{
    TmdbId showId(config().identifier.showIdentifier);

    if (!showId.isValid()) {
        qCWarning(generic) << "[TmdbTvEpisodeScrapeJob] Invalid TMDB ID for TV show, cannot scrape episode!";
        ScraperError configError;
        configError.error = ScraperError::Type::ConfigError;
        configError.message = tr("TMDB show ID is invalid! Cannot load requested episode.");
        setScraperError(configError);
        QTimer::singleShot(0, this, [this]() { emitFinished(); });
        return;
    }

    qCInfo(generic) << "[TmdbTvEpisodeScrapeJob] Have to load season first.";

    m_api.loadEpisode(config().locale,
        showId,
        config().identifier.seasonNumber,
        config().identifier.episodeNumber,
        [this](QJsonDocument json, ScraperError error) {
            if (!error.hasError()) {
                TmdbTvEpisodeParser::parseInfos(m_api, episode(), json.object());
            }
            setScraperError(error);
            emitFinished();
        });
}

} // namespace scraper
} // namespace mediaelch
