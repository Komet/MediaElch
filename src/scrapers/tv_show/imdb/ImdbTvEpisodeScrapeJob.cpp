#include "scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"
#include "log/Log.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

ImdbTvEpisodeScrapeJob::ImdbTvEpisodeScrapeJob(ImdbApi& api, EpisodeScrapeJob::Config _config, QObject* parent) :
    EpisodeScrapeJob(_config, parent), m_api{api}
{
}

void ImdbTvEpisodeScrapeJob::doStart()
{
    if (config().identifier.hasEpisodeIdentifier()) {
        loadEpisode(ImdbId(config().identifier.episodeIdentifier));
    } else {
        loadSeason();
    }
}

void ImdbTvEpisodeScrapeJob::loadSeason()
{
    qCDebug(generic) << "[ImdbTvEpisodeScrapeJob] Have to load season first.";

    ImdbId showId(config().identifier.showIdentifier);

    if (!showId.isValid()) {
        qCWarning(generic) << "[ImdbTvEpisodeScrapeJob] Invalid IMDb ID for TV show, cannot scrape episode!";
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("Neither IMDb show ID nor episode ID are valid! Cannot load requested episode.");
        setScraperError(error);
        emitFinished();
        return;
    }

    // The episode parser requires season/episode to be set when
    // calling parseIdFromSeason()
    episode().setSeason(config().identifier.seasonNumber);
    episode().setEpisode(config().identifier.episodeNumber);

    m_api.loadSeason(
        config().locale, showId, config().identifier.seasonNumber, [this, showId](QString html, ScraperError error) {
            if (error.hasError()) {
                setScraperError(error);
                emitFinished();
                return;
            }
            ImdbTvEpisodeParser::parseIdFromSeason(episode(), html);
            if (!episode().imdbId().isValid()) {
                qCWarning(generic) << "[ImdbTvEpisodeScrapeJob] Could not parse IMDb ID for episode from season page! "
                                   << episode().seasonNumber() << episode().episodeNumber();
                ScraperError configError;
                configError.error = ScraperError::Type::ConfigError;
                configError.message =
                    tr("IMDb ID could not be loaded from season page! Cannot load requested episode.");
                setScraperError(configError);
                emitFinished();
            } else {
                loadEpisode(episode().imdbId());
            }
        });
}

void ImdbTvEpisodeScrapeJob::loadEpisode(const ImdbId& episodeId)
{
    if (!episodeId.isValid()) {
        qCWarning(generic) << "[ImdbTvEpisodeScrapeJob] Invalid IMDb ID, cannot scrape episode!";
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("IMDb ID is invalid! Cannot load requested episode.");
        setScraperError(error);
        emitFinished();
        return;
    }

    qCInfo(generic) << "[ImdbTvEpisodeScrapeJob] Loading episode with IMDb ID" << episodeId.toString();
    m_api.loadTitle(config().locale, episodeId, ImdbApi::PageKind::Reference, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
        } else if (html.isEmpty()) {
            qCWarning(generic) << "[ImdbTvEpisodeScrapeJob] Empty episode HTML!";
            ScraperError networkError;
            networkError.error = ScraperError::Type::NetworkError;
            networkError.message = tr("Loaded IMDb content is empty. Cannot load requested episode.");
            setScraperError(networkError);
        } else {
            ImdbTvEpisodeParser::parseInfos(episode(), html, config().locale);
        }
        emitFinished();
    });
}

} // namespace scraper
} // namespace mediaelch
