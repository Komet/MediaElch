#include "scrapers/tv_show/ShowScrapeJob.h"

#include "data/tv_show/TvShow.h"

namespace mediaelch {
namespace scraper {

ShowScrapeJob::ShowScrapeJob(ShowScrapeJob::Config config, QObject* parent) :
    worker::Job(parent), m_tvShow{new TvShow({}, this)}, m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit loadFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of ShowSearchJob are updated.
    setAutoDelete(false);
}

const ScraperError& ShowScrapeJob::scraperError() const
{
    return m_scraperError;
}

void ShowScrapeJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}

} // namespace scraper
} // namespace mediaelch
