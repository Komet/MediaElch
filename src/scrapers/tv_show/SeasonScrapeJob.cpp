#include "scrapers/tv_show/SeasonScrapeJob.h"

namespace mediaelch {
namespace scraper {

SeasonScrapeJob::SeasonScrapeJob(SeasonScrapeJob::Config config, QObject* parent) :
    QObject(parent), m_config{std::move(config)}
{
}

bool SeasonScrapeJob::hasError() const
{
    return m_error.hasError();
}

const ScraperError& SeasonScrapeJob::error() const
{
    return m_error;
}


} // namespace scraper
} // namespace mediaelch
