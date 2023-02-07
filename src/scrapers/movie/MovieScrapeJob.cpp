#include "scrapers/movie/MovieScrapeJob.h"

#include "data/movie/Movie.h"

namespace mediaelch {
namespace scraper {

MovieScrapeJob::MovieScrapeJob(MovieScrapeJob::Config config, QObject* parent) :
    worker::Job(parent), m_movie{new Movie({}, this)}, m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit loadFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of MovieScrapeJob are updated.
    setAutoDelete(false);
}

const ScraperError& MovieScrapeJob::scraperError() const
{
    return m_scraperError;
}

void MovieScrapeJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}

} // namespace scraper
} // namespace mediaelch
