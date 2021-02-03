#include "scrapers/movie/MovieMerger.h"

#include "movies/Movie.h"

#include <QDebug>

namespace mediaelch {
namespace scraper {

static void copyDetailToMovie(Movie& target, const Movie& source, MovieScraperInfo detail)
{
    if (source.tmdbId().isValid()) {
        target.setTmdbId(source.tmdbId());
    }
    if (source.imdbId().isValid()) {
        target.setImdbId(source.imdbId());
    }

    switch (detail) {
    case MovieScraperInfo::Invalid: {
        qCritical() << "[MovieMerger] Cannot copy details 'invalid'";
        break;
    }
    case MovieScraperInfo::Title: {
        target.setName(source.name());
        break;
    }
    case MovieScraperInfo::Tagline: {
        target.setTagline(source.tagline());
        break;
    }
    case MovieScraperInfo::Rating: {
        target.ratings() << source.ratings();
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
    case MovieScraperInfo::Overview: {
        target.setOverview(source.overview());
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
        const auto& sourceActors = source.actors();
        for (const Actor* actor : sourceActors) {
            target.addActor(*actor);
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
        // TODO ??
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
        // TODO
        break;
    }
    case MovieScraperInfo::Thumb: {
        // TODO
        break;
    }
    }
}

void copyDetailsToMovie(Movie& target, const Movie& source, const QSet<MovieScraperInfo>& details)
{
    for (MovieScraperInfo detail : details) {
        copyDetailToMovie(target, source, detail);
    }
}

} // namespace scraper
} // namespace mediaelch
