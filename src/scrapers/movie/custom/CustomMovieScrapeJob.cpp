#include "scrapers/movie/custom/CustomMovieScrapeJob.h"

#include "movies/Movie.h"

namespace mediaelch {
namespace scraper {

CustomMovieScrapeJob::CustomMovieScrapeJob(MovieScrapeJob::Config _config, QObject* parent) :
    MovieScrapeJob(std::move(_config), parent)
{
}

void CustomMovieScrapeJob::execute()
{
  // TODO ANDRE
}

} // namespace scraper
} // namespace mediaelch
