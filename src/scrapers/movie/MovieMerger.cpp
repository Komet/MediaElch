#include "scrapers/movie/MovieMerger.h"

#include "data/movie/Movie.h"
#include "log/Log.h"

namespace mediaelch {
namespace scraper {

namespace {

// TODO: Option "only replace if source has value"
void copyDetailToMovie(Movie& target,
    const Movie& source,
    MovieScraperInfo detail,
    bool usePlotForOutline,
    bool ignoreDuplicateOriginalTitle)
{
    if (source.tmdbId().isValid()) {
        target.setTmdbId(source.tmdbId());
    }
    if (source.imdbId().isValid()) {
        target.setImdbId(source.imdbId());
    }

    switch (detail) {
    case MovieScraperInfo::Invalid: {
        qCCritical(generic) << "[MovieMerger] Cannot copy details 'invalid'";
        break;
    }
    case MovieScraperInfo::Title: {
        target.setName(source.name());
        if (!ignoreDuplicateOriginalTitle || source.name() != source.originalName()) {
            target.setOriginalName(source.originalName());
        }
        break;
    }
    case MovieScraperInfo::Tagline: {
        target.setTagline(source.tagline());
        break;
    }
    case MovieScraperInfo::Rating: {
        target.ratings().merge(source.ratings());
        break;
    }
    case MovieScraperInfo::Released: {
        target.setReleased(source.released());
        break;
    }
    case MovieScraperInfo::Runtime: {
        target.setRuntime(source.runtime());
        break;
    }
    case MovieScraperInfo::Certification: {
        target.setCertification(source.certification());
        break;
    }
    case MovieScraperInfo::Trailer: {
        target.setTrailer(source.trailer());
        break;
    }
    case MovieScraperInfo::TvShowLinks: {
        target.setTvShowLinks(source.tvShowLinks());
        break;
    }
    case MovieScraperInfo::Overview: {
        target.setOverview(source.overview());
        if (!source.outline().isEmpty()) {
            target.setOutline(source.outline());
        } else if (usePlotForOutline) {
            target.setOutline(source.overview());
        }
        break;
    }
    case MovieScraperInfo::Poster: {
        const auto& sourceImages = source.constImages().posters();
        for (const Poster& poster : sourceImages) {
            target.images().addPoster(poster);
        }
        break;
    }
    case MovieScraperInfo::Backdrop: {
        const auto& sourceImages = source.constImages().backdrops();
        for (const Poster& backdrop : sourceImages) {
            target.images().addBackdrop(backdrop);
        }
        break;
    }
    case MovieScraperInfo::Actors: {
        // Simple brute-force merge.
        // \todo This code can most likely be simplified
        const auto& sourceActors = source.actors();
        const auto& targetActors = target.actors();
        for (const Actor* sourceActor : sourceActors) {
            bool hasActor = false;
            for (Actor* targetActor : targetActors) {
                if (targetActor->name == sourceActor->name) {
                    targetActor->thumb = sourceActor->thumb;
                    hasActor = true;
                    break;
                }
            }
            if (!hasActor) {
                target.addActor(*sourceActor);
            }
        }

        break;
    }
    case MovieScraperInfo::Genres: {
        const auto& genres = source.genres();
        for (const auto& genre : genres) {
            target.addGenre(genre);
        }
        break;
    }
    case MovieScraperInfo::Studios: {
        const auto& studios = source.studios();
        for (const auto& studio : studios) {
            target.addStudio(studio);
        }
        break;
    }
    case MovieScraperInfo::Countries: {
        const auto& countries = source.countries();
        for (const auto& country : countries) {
            target.addCountry(country);
        }
        break;
    }
    case MovieScraperInfo::Writer: {
        target.setWriter(source.writer());
        break;
    }
    case MovieScraperInfo::Director: {
        target.setDirector(source.director());
        break;
    }
    case MovieScraperInfo::Tags: {
        const auto& tags = source.tags();
        for (const auto& tag : tags) {
            target.addTag(tag);
        }
        break;
    }
    case MovieScraperInfo::ExtraFanarts: {
        // no-op: Are loaded from disk only.
        break;
    }
    case MovieScraperInfo::Set: {
        target.setSet(source.set());
        break;
    }
    case MovieScraperInfo::Logo: {
        const auto& logos = source.constImages().logos();
        for (const auto& logo : logos) {
            target.images().addLogo(logo);
        }
        break;
    }
    case MovieScraperInfo::CdArt: {
        const auto& images = source.constImages().discArts();
        for (const auto& discArt : images) {
            target.images().addDiscArt(discArt);
        }
        break;
    }
    case MovieScraperInfo::ClearArt: {
        const auto& images = source.constImages().clearArts();
        for (const auto& clearArt : images) {
            target.images().addClearArt(clearArt);
        }
        break;
    }
    case MovieScraperInfo::Banner: {
        const QByteArray banner = source.constImages().image(ImageType::MovieBanner);
        if (!banner.isEmpty()) {
            target.images().setImage(ImageType::MovieBanner, banner);
        }
        break;
    }
    case MovieScraperInfo::Thumb: {
        const QByteArray thumb = source.constImages().image(ImageType::MovieThumb);
        if (!thumb.isEmpty()) {
            target.images().setImage(ImageType::MovieThumb, thumb);
        }
        break;
    }
    }
}

} // namespace

void copyDetailsToMovie(Movie& target,
    const Movie& source,
    const QSet<MovieScraperInfo>& details,
    bool usePlotForOutline,
    bool ignoreDuplicateOriginalTitle)
{
    const bool wasBlocked = target.blockSignals(true);
    for (MovieScraperInfo detail : details) {
        copyDetailToMovie(target, source, detail, usePlotForOutline, ignoreDuplicateOriginalTitle);
    }
    target.blockSignals(wasBlocked);
}

} // namespace scraper
} // namespace mediaelch
