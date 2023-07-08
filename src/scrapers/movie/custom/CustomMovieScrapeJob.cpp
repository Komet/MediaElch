#include "scrapers/movie/custom/CustomMovieScrapeJob.h"

#include "data/movie/Movie.h"
#include "scrapers/movie/MovieMerger.h"
#include "scrapers/movie/MovieScraper.h"
#include "settings/Settings.h"

namespace {

static QSet<MovieScraperInfo> combineScraperDetails(
    const mediaelch::scraper::CustomMovieScrapeJob::CustomScraperConfig& config)
{
    // Ensure that config.details is actually a superset of all scraper details.
    QSet<MovieScraperInfo> superset;
    for (auto i = config.scraperMap.begin(); i != config.scraperMap.end(); ++i) {
        superset.unite(i.value().details);
    }
    return superset;
}

} // namespace

namespace mediaelch {
namespace scraper {

CustomMovieScrapeJob::CustomMovieScrapeJob(CustomScraperConfig _config, QObject* parent) :
    MovieScrapeJob(MovieScrapeJob::Config{MovieIdentifier{""}, Locale::NoLocale, combineScraperDetails(_config)},
        parent),
    m_customScraperConfig{std::move(_config)}
{
}

void CustomMovieScrapeJob::doStart()
{
    if (m_customScraperConfig.scraperMap.isEmpty()) {
        emitFinished();
        return;
    }

    for (auto i = m_customScraperConfig.scraperMap.begin(); i != m_customScraperConfig.scraperMap.end(); ++i) {
        MovieScrapeJob* job = i.key()->loadMovie(i.value());
        connect(job, &MovieScrapeJob::loadFinished, this, &CustomMovieScrapeJob::onScraperFinished);
        m_jobs.push_back(job);
        job->start();
    }
}

void CustomMovieScrapeJob::onScraperFinished(MovieScrapeJob* scrapeJob)
{
    copyDetailsToMovie(*m_movie,
        scrapeJob->movie(),
        scrapeJob->config().details,
        Settings::instance()->usePlotForOutline(),
        Settings::instance()->ignoreDuplicateOriginalTitle());
    scrapeJob->deleteLater();

    const bool isRemoved = m_jobs.removeOne(scrapeJob);
    // If it does not exist, we have either multiple signals for the same job or other bugs.
    MediaElch_Assert(isRemoved);

    if (m_jobs.isEmpty()) {
        emitFinished();
    }
}

} // namespace scraper
} // namespace mediaelch
