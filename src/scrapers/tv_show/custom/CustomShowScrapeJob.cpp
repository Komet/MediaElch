#include "scrapers/tv_show/custom/CustomShowScrapeJob.h"

#include "globals/Manager.h"
#include "log/Log.h"
#include "scrapers/tv_show/ShowMerger.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"
#include "utils/Containers.h"

#include <QMutexLocker>
#include <utility>

namespace mediaelch {
namespace scraper {

CustomShowScrapeJob::CustomShowScrapeJob(CustomTvScraperConfiguration& customConfig,
    ShowScrapeJob::Config config,
    QObject* parent) :
    ShowScrapeJob(config, parent), m_customConfig{customConfig}
{
}

void CustomShowScrapeJob::doStart()
{
    // Because the custom TV scraper always starts with TMDB, the query should stay the same but
    // we have to correctly set the details that we want to load from TmdbTv.
    ShowScrapeJob::Config tmdbConfig = configFor(TmdbTv::ID, config().identifier);

    if (tmdbConfig.details.isEmpty()) {
        // HACK: in onTmdbLoaded() we copy details to this job's show.
        //       But if we do not load any details from TMDB, we don't copy anything
        //       not even the IDs that are needed for other scrapers, etc.
        //       By using this hack, we always invoke copyDetailsToShow() so that IDs are copied.
        tmdbConfig.details.insert(ShowScraperInfo::Invalid);
    }

    auto* tmdbJob = m_customConfig.tmdbTv->loadShow(tmdbConfig);
    connect(tmdbJob, &TmdbTvShowScrapeJob::loadFinished, this, &CustomShowScrapeJob::onTmdbLoaded);
    tmdbJob->start();
}

void CustomShowScrapeJob::onTmdbLoaded(ShowScrapeJob* job)
{
    copyDetailsToShow(tvShow(), job->tvShow(), job->config().details);
    job->deleteLater();

    const QStringList scrapersToUse = m_customConfig.scraperForShowDetails.values();
    const bool loadImdb = tvShow().imdbId().isValid() && scrapersToUse.contains(ImdbTv::ID);

    m_loadCounter = 1;

    if (loadImdb) {
        ++m_loadCounter;
    }

    if (loadImdb) {
        loadWithScraper(ImdbTv::ID, ShowIdentifier(tvShow().imdbId()));
    }

    decreaseCounterAndCheckIfFinished();
}

void CustomShowScrapeJob::loadWithScraper(const QString& scraperId, const ShowIdentifier& identifier)
{
    ShowScrapeJob::Config scraperConfig = configFor(scraperId, identifier);
    if (scraperConfig.details.isEmpty()) {
        // No need to load from scraper if no details are requested.
        decreaseCounterAndCheckIfFinished();
        return;
    }

    TvScraper* scraper = m_customConfig.scraperForId(scraperId);
    if (scraper == nullptr) {
        qCCritical(generic) << "[CustomShowScrapeJob] Invalid scraper ID for custom tv scraper:" << scraperId;
        decreaseCounterAndCheckIfFinished();
        return;
    }

    auto* scrapeJob = scraper->loadShow(scraperConfig);
    connect(scrapeJob, &ShowScrapeJob::loadFinished, this, [this](ShowScrapeJob* job) {
        {
            // locking to avoid concurrent access to m_episodes
            QMutexLocker locker(&m_loadMutex);
            copyDetailsToShow(tvShow(), job->tvShow(), job->config().details);
        }
        job->deleteLater();
        decreaseCounterAndCheckIfFinished();
    });
    scrapeJob->start();
}

void CustomShowScrapeJob::decreaseCounterAndCheckIfFinished()
{
    QMutexLocker locker(&m_loadMutex);
    --m_loadCounter;
    if (m_loadCounter <= 0) {
        locker.unlock();
        emitFinished();
    }
}

ShowScrapeJob::Config CustomShowScrapeJob::configFor(const QString& scraperId, const ShowIdentifier& id)
{
    ShowScrapeJob::Config scraperConfig = config();
    scraperConfig.locale = localeFor(scraperId);
    scraperConfig.identifier = id;

    auto detailsForScraper = mediaelch::listToSet(m_customConfig.scraperForShowDetails.keys(scraperId));
    detailsForScraper.intersect(scraperConfig.details);
    scraperConfig.details = detailsForScraper;

    return scraperConfig;
}

Locale CustomShowScrapeJob::localeFor(const QString& scraperId) const
{
    return Settings::instance()->value({"scrapers", QStringLiteral("Scrapers/%1/Language").arg(scraperId)}).toString();
}

} // namespace scraper
} // namespace mediaelch
