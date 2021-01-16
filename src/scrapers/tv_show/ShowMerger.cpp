#include "scrapers/tv_show/ShowMerger.h"

#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

#include <QDebug>

namespace mediaelch {
namespace scraper {

static void copyDetailToShow(TvShow& target, TvShow& source, ShowScraperInfo detail)
{
    if (source.tmdbId().isValid()) {
        target.setTmdbId(source.tmdbId());
    }
    if (source.tvdbId().isValid()) {
        target.setTvdbId(source.tvdbId());
    }
    if (source.imdbId().isValid()) {
        target.setImdbId(source.imdbId());
    }
    if (source.tvmazeId().isValid()) {
        target.setTvMazeId(source.tvmazeId());
    }
    switch (detail) {
    case ShowScraperInfo::Invalid: qCritical() << "[ShowMerger] Cannot copy details 'invalid'"; break;
    case ShowScraperInfo::Title: {
        target.setTitle(source.title());
        target.setOriginalTitle(source.originalTitle());
        // sort title not merged because scrapers don't provide it
        break;
    }
    case ShowScraperInfo::Actors: {
        const auto& actors = source.actors();
        for (const auto* actor : actors) {
            target.addActor(*actor);
        }
        break;
    }
    case ShowScraperInfo::Banner: {
        const auto& banners = source.banners();
        for (const Poster& banner : banners) {
            target.addBanner(banner);
        }
        break;
    }
    case ShowScraperInfo::Certification: {
        target.setCertification(source.certification());
        break;
    }
    case ShowScraperInfo::Fanart: {
        const auto& backdrops = source.backdrops();
        for (const Poster& backdrop : backdrops) {
            target.addBackdrop(backdrop);
        }
        break;
    }
    case ShowScraperInfo::FirstAired: {
        target.setFirstAired(source.firstAired());
        break;
    }
    case ShowScraperInfo::Genres: {
        const auto genres = source.genres();
        for (const QString& genre : genres) {
            target.addGenre(genre);
        }
        break;
    }
    case ShowScraperInfo::Network: {
        target.setNetwork(source.network());
        break;
    }
    case ShowScraperInfo::Overview: {
        target.setOverview(source.overview());
        break;
    }
    case ShowScraperInfo::Poster: {
        const auto& posters = source.posters();
        for (const Poster& poster : posters) {
            target.addPoster(poster);
        }
        break;
    }
    case ShowScraperInfo::Rating: {
        target.ratings().append(source.ratings());
        break;
    }
    case ShowScraperInfo::SeasonPoster: {
        const auto& copy = source.allSeasonPosters();
        for (auto i = copy.constBegin(); i != copy.constEnd(); ++i) {
            for (const Poster& poster : i.value()) {
                target.addSeasonPoster(i.key(), poster);
            }
        }
        break;
    }
    case ShowScraperInfo::SeasonBackdrop: {
        const auto& copy = source.allSeasonBackdrops();
        for (auto i = copy.constBegin(); i != copy.constEnd(); ++i) {
            for (const Poster& poster : i.value()) {
                target.addSeasonBackdrop(i.key(), poster);
            }
        }
        break;
    }
    case ShowScraperInfo::SeasonBanner: {
        const auto& copy = source.allSeasonBanners();
        for (auto i = copy.constBegin(); i != copy.constEnd(); ++i) {
            for (const Poster& poster : i.value()) {
                target.addSeasonBanner(i.key(), poster);
            }
        }
        break;
    }
    case ShowScraperInfo::SeasonThumb: {
        const auto& copy = source.allSeasonThumbs();
        for (auto i = copy.constBegin(); i != copy.constEnd(); ++i) {
            for (const Poster& poster : i.value()) {
                target.addSeasonThumb(i.key(), poster);
            }
        }
        break;
    }
    case ShowScraperInfo::ExtraArts:
    case ShowScraperInfo::ExtraFanarts: {
        // no-op: Are loaded from disk only.
        break;
    }
    case ShowScraperInfo::Tags: {
        const auto& tags = source.tags();
        for (const QString& tag : tags) {
            target.addTag(tag);
        }
        break;
    }
    case ShowScraperInfo::Thumb: {
        // not implemented in TV show
        break;
    }
    case ShowScraperInfo::Runtime: {
        target.setRuntime(source.runtime());
        break;
    }
    case ShowScraperInfo::Status: {
        target.setStatus(source.status());
        break;
    }
    }
}

static void copyDetailToEpisode(TvShowEpisode& target, const TvShowEpisode& source, EpisodeScraperInfo detail)
{
    if (source.tmdbId().isValid()) {
        target.setTmdbId(source.tmdbId());
    }
    if (source.tvdbId().isValid()) {
        target.setTvdbId(source.tvdbId());
    }
    if (source.imdbId().isValid()) {
        target.setImdbId(source.imdbId());
    }
    if (source.tvmazeId().isValid()) {
        target.setTvMazeId(source.tvmazeId());
    }
    switch (detail) {
    case EpisodeScraperInfo::Invalid: {
        qCritical() << "[ShowMerger] Cannot copy details 'invalid'";
        break;
    }
    case EpisodeScraperInfo::Actors: {
        const auto& actors = source.actors();
        for (const auto* actor : actors) {
            target.addActor(*actor);
        }
        break;
    }
    case EpisodeScraperInfo::Certification: {
        target.setCertification(source.certification());
        break;
    }
    case EpisodeScraperInfo::Director: {
        QStringList combinedDirectors = target.directors();
        combinedDirectors.append(source.directors());
        target.setDirectors(combinedDirectors);
        break;
    }
    case EpisodeScraperInfo::Writer: {
        QStringList combinedWriters = target.writers();
        combinedWriters.append(source.writers());
        target.setWriters(combinedWriters);
        break;
    }
    case EpisodeScraperInfo::FirstAired: {
        target.setFirstAired(source.firstAired());
        break;
    }
    case EpisodeScraperInfo::Network: {
        target.setNetwork(source.network());
        break;
    }
    case EpisodeScraperInfo::Overview: {
        target.setOverview(source.overview());
        break;
    }
    case EpisodeScraperInfo::Rating: {
        auto combinedRatings = target.ratings();
        combinedRatings.append(source.ratings());
        target.ratings() = combinedRatings;
        break;
    }
    case EpisodeScraperInfo::Tags: {
        const auto& tags = source.tags();
        for (const QString& tag : tags) {
            target.addTag(tag);
        }
        break;
    }
    case EpisodeScraperInfo::Thumbnail: {
        target.setThumbnail(source.thumbnail());
        break;
    }
    case EpisodeScraperInfo::Title: {
        target.setTitle(source.title());
        break;
    }
    }
}

void copyDetailsToShow(TvShow& target, TvShow& source, const QSet<ShowScraperInfo>& details)
{
    for (ShowScraperInfo detail : details) {
        copyDetailToShow(target, source, detail);
    }
}

void copyDetailsToEpisode(TvShowEpisode& target, const TvShowEpisode& source, const QSet<EpisodeScraperInfo>& details)
{
    if (details.contains(EpisodeScraperInfo::Thumbnail)) {
        target.setWantThumbnailDownload(true);
    }
    for (EpisodeScraperInfo detail : details) {
        copyDetailToEpisode(target, source, detail);
    }
}

void copyDetailsToShowEpisodes(TvShow& target,
    const EpisodeMap& source,
    bool onlyCopyNew,
    const QSet<EpisodeScraperInfo>& details)
{
    for (auto* episode : target.episodes()) {
        // Either all episodes should be merged or those who don't have all details, yet.
        if (onlyCopyNew && episode->infoLoaded()) {
            continue;
        }

        auto* scraped = source[{episode->seasonNumber(), episode->episodeNumber()}];
        if (scraped != nullptr) {
            copyDetailsToEpisode(*episode, *scraped, details);
        } else if (!episode->isDummy()) {
            qCritical() << "[TvShow] Cannot merge episode that wasn't scraped. This should not happen! For:"
                        << episode->seasonNumber() << "," << episode->episodeNumber();
        }
    }
}

void copyDetailsToEpisodeMap(EpisodeMap& target,
    const EpisodeMap& source,
    const QSet<EpisodeScraperInfo>& details,
    QObject* parentForNewEpisodes)
{
    for (auto sourceIterator = source.constBegin(); sourceIterator != source.constEnd(); ++sourceIterator) {
        if (!target.contains(sourceIterator.key())) {
            target.insert(sourceIterator.key(), new TvShowEpisode({}, parentForNewEpisodes));
        }

        TvShowEpisode* targetEpisode = target.value(sourceIterator.key());
        const TvShowEpisode* sourceEpisode = sourceIterator.value();
        if (targetEpisode != nullptr && sourceEpisode != nullptr) {
            copyDetailsToEpisode(*targetEpisode, *sourceEpisode, details);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
