#include "scrapers/tv_show/EpisodeScrapeJob.h"

#include "tv_shows/TvShowEpisode.h"

namespace mediaelch {
namespace scraper {


EpisodeScrapeJob::EpisodeScrapeJob(EpisodeScrapeJob::Config config, QObject* parent) :
    QObject(parent), m_episode{new TvShowEpisode({}, this)}, m_config{std::move(config)}
{
}

bool EpisodeScrapeJob::hasError() const
{
    return m_error.hasError();
}

const ScraperError& EpisodeScrapeJob::error() const
{
    return m_error;
}


} // namespace scraper
} // namespace mediaelch
