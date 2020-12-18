#include "scrapers/tv_show/ShowScrapeJob.h"

#include "tv_shows/TvShow.h"

namespace mediaelch {
namespace scraper {

ShowScrapeJob::ShowScrapeJob(ShowScrapeJob::Config config, QObject* parent) :
    QObject(parent), m_tvShow{new TvShow({}, this)}, m_config{std::move(config)}
{
}

bool ShowScrapeJob::hasError() const
{
    return m_error.hasError();
}

const ScraperError& ShowScrapeJob::error() const
{
    return m_error;
}

} // namespace scraper
} // namespace mediaelch
