#include "scrapers/tv_show/EpisodeScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"

namespace mediaelch {
namespace scraper {


EpisodeScrapeJob::EpisodeScrapeJob(EpisodeScrapeJob::Config config, QObject* parent) :
    worker::Job(parent), m_episode{new TvShowEpisode({}, this)}, m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit loadFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of ShowSearchJob are updated.
    setAutoDelete(false);
}

const ScraperError& EpisodeScrapeJob::scraperError() const
{
    return m_scraperError;
}

void EpisodeScrapeJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}


} // namespace scraper
} // namespace mediaelch
