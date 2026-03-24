#include "scrapers/tv_show/imdb/ImdbTvSeasonScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"
#include "log/Log.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/imdb/ImdbJsonParser.h"
#include "utils/Containers.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

ImdbTvSeasonScrapeJob::ImdbTvSeasonScrapeJob(ImdbApi& api, SeasonScrapeJob::Config _config, QObject* parent) :
    SeasonScrapeJob(_config, parent), m_api{api}, m_showId{ImdbId(config().showIdentifier.str())}
{
}

void ImdbTvSeasonScrapeJob::doStart()
{
    if (!m_showId.isValid()) {
        qCWarning(generic) << "[ImdbTv] Provided IMDb id is invalid:" << config().showIdentifier;
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("Show is missing an IMDb id");
        setScraperError(error);
        QTimer::singleShot(0, this, [this]() { emitFinished(); });
        return;
    }

    loadEpisodes();
}

void ImdbTvSeasonScrapeJob::loadEpisodes()
{
    // Load all episodes in bulk via GraphQL — one request for up to 250 episodes.
    // This replaces the old sequential per-episode loading pattern.
    m_api.loadEpisodesViaGraphQL(m_showId, 250, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }
        parseAndStoreEpisodes(data);
        emitFinished();
    });
}

void ImdbTvSeasonScrapeJob::parseAndStoreEpisodes(const QString& json)
{
    const QVector<ImdbEpisodeData> episodes = ImdbJsonParser::parseEpisodesFromGraphQL(json);

    for (const ImdbEpisodeData& epData : episodes) {
        const SeasonNumber season(epData.seasonNumber);
        const EpisodeNumber epNum(epData.episodeNumber);

        // Skip episodes from seasons we didn't request (unless loading all)
        if (!config().shouldLoadAllSeasons() && !config().seasons.contains(season)) {
            continue;
        }

        auto* episode = new TvShowEpisode({}, this);
        episode->setSeason(season);
        episode->setEpisode(epNum);
        episode->setImdbId(epData.imdbId);

        if (epData.title.hasValue()) {
            episode->setTitle(epData.title.value);
        }
        if (epData.overview.hasValue()) {
            episode->setOverview(epData.overview.value);
        }
        if (epData.firstAired.hasValue()) {
            episode->setFirstAired(epData.firstAired.value);
        }
        if (epData.thumbnail.hasValue()) {
            episode->setThumbnail(epData.thumbnail.value.thumbUrl);
        }
        for (const Rating& rating : epData.ratings) {
            episode->ratings().addRating(rating);
        }
        if (epData.runtime.hasValue()) {
            // TvShowEpisode doesn't have setRuntime — runtime is only on TvShow level
        }
        if (epData.certification.hasValue()) {
            episode->setCertification(epData.certification.value);
        }
        if (!epData.directors.isEmpty()) {
            episode->setDirectors(setToStringList(epData.directors));
        }
        if (!epData.writers.isEmpty()) {
            episode->setWriters(setToStringList(epData.writers));
        }
        for (const Actor& actor : epData.actors) {
            episode->addActor(actor);
        }

        storeEpisode(episode);
    }
}

void ImdbTvSeasonScrapeJob::storeEpisode(TvShowEpisode* episode)
{
    const SeasonNumber season = episode->seasonNumber();
    if (config().shouldLoadAllSeasons() || config().seasons.contains(season)) {
        m_episodes[{season, episode->episodeNumber()}] = episode;
    } else {
        episode->deleteLater();
    }
}

} // namespace scraper
} // namespace mediaelch
