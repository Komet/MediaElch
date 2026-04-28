#include "scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"
#include "log/Log.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/imdb/ImdbJsonParser.h"
#include "utils/Containers.h"

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
        loadFromSeason();
    }
}

void ImdbTvEpisodeScrapeJob::loadFromSeason()
{
    qCDebug(generic) << "[ImdbTvEpisodeScrapeJob] Loading episode via season bulk query.";

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

    episode().setSeason(config().identifier.seasonNumber);
    episode().setEpisode(config().identifier.episodeNumber);

    // Load episodes for the specific season via GraphQL and find the one we need
    m_api.loadSeasonEpisodesViaGraphQL(
        showId, config().identifier.seasonNumber.toInt(), 250, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }

        const QVector<ImdbEpisodeData> episodes = ImdbJsonParser::parseEpisodesFromGraphQL(data, config().locale);
        const int targetSeason = config().identifier.seasonNumber.toInt();
        const int targetEpisode = config().identifier.episodeNumber.toInt();

        for (const ImdbEpisodeData& epData : episodes) {
            if (epData.seasonNumber == targetSeason && epData.episodeNumber == targetEpisode) {
                // Found our episode — load its full details via individual GraphQL query
                loadEpisode(epData.imdbId);
                return;
            }
        }

        qCWarning(generic) << "[ImdbTvEpisodeScrapeJob] Could not find episode S" << targetSeason << "E"
                           << targetEpisode << "in GraphQL response";
        ScraperError notFoundError;
        notFoundError.error = ScraperError::Type::ConfigError;
        notFoundError.message = tr("Episode not found in season listing.");
        setScraperError(notFoundError);
        emitFinished();
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
    m_api.loadTitleViaGraphQL(episodeId, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
        } else if (data.isEmpty()) {
            qCWarning(generic) << "[ImdbTvEpisodeScrapeJob] Empty GraphQL response!";
            ScraperError networkError;
            networkError.error = ScraperError::Type::NetworkError;
            networkError.message = tr("Loaded IMDb content is empty. Cannot load requested episode.");
            setScraperError(networkError);
        } else {
            parseAndAssignInfos(data);
        }
        emitFinished();
    });
}

void ImdbTvEpisodeScrapeJob::parseAndAssignInfos(const QString& json)
{
    ImdbData data = ImdbJsonParser::parseFromGraphQL(json, config().locale);

    if (data.imdbId.isValid()) {
        episode().setImdbId(data.imdbId);
    }
    if (data.title.hasValue()) {
        episode().setTitle(data.title.value);
    }
    if (data.overview.hasValue()) {
        episode().setOverview(data.overview.value);
    }
    if (data.released.hasValue()) {
        episode().setFirstAired(data.released.value);
    }
    for (const Rating& rating : data.ratings) {
        episode().ratings().addRating(rating);
    }
    if (data.certification.hasValue()) {
        episode().setCertification(data.certification.value);
    }
    if (data.poster.hasValue()) {
        episode().setThumbnail(data.poster.value.thumbUrl);
    }
    if (!data.directors.isEmpty()) {
        episode().setDirectors(setToStringList(data.directors));
    }
    if (!data.writers.isEmpty()) {
        episode().setWriters(setToStringList(data.writers));
    }
    for (const Actor& actor : data.actors) {
        episode().addActor(actor);
    }
}

} // namespace scraper
} // namespace mediaelch
