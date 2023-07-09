#include "ui/movies/MoviePreviewAdapter.h"

namespace mediaelch {

MoviePreviewAdapter::~MoviePreviewAdapter() = default;

std::unique_ptr<ScrapePreview::JobAdapter>
MoviePreviewAdapter::createFor(scraper::MovieScraper* scraper, scraper::MovieIdentifier id, Locale locale)
{
    scraper::MovieScrapeJob::Config config;
    config.identifier = std::move(id);
    config.locale = std::move(locale);
    config.details = {MovieScraperInfo::Title, MovieScraperInfo::Overview, MovieScraperInfo::Poster};

    scraper::MovieScrapeJob* job = scraper->loadMovie(config);
    return std::make_unique<MoviePreviewAdapter>(job);
}

} // namespace mediaelch
