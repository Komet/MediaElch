#include "scrapers/tv_show/SeasonScrapeJob.h"

namespace mediaelch {
namespace scraper {

SeasonScrapeJob::SeasonScrapeJob(SeasonScrapeJob::Config config, QObject* parent) :
    worker::Job(parent), m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit loadFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of ShowSearchJob are updated.
    setAutoDelete(false);
}

void SeasonScrapeJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}

const ScraperError& SeasonScrapeJob::scraperError() const
{
    return m_scraperError;
}


} // namespace scraper
} // namespace mediaelch
