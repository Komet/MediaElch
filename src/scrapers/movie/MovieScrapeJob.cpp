#include "scrapers/movie/MovieScrapeJob.h"

#include "movies/Movie.h"

namespace mediaelch {
namespace scraper {

MovieScrapeJob::MovieScrapeJob(MovieScrapeJob::Config config, QObject* parent) :
    QObject(parent), m_movie{new Movie({}, this)}, m_config{std::move(config)}
{
}

bool MovieScrapeJob::hasError() const
{
    return m_error.hasError();
}

const ScraperError& MovieScrapeJob::error() const
{
    return m_error;
}

} // namespace scraper
} // namespace mediaelch
