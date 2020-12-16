#include "scrapers/tv_show/custom/CustomShowScrapeJob.h"

#include "globals/Containers.h"
#include "globals/Manager.h"
#include "scrapers/tv_show/ShowMerger.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/thetvdb/TheTvDbShowScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"

#include <QMutexLocker>
#include <utility>

namespace mediaelch {
namespace scraper {

CustomShowScrapeJob::CustomShowScrapeJob(CustomTvScraperConfig customConfig,
    ShowScrapeJob::Config config,
    QObject* parent) :
    ShowScrapeJob(config, parent), m_customConfig{std::move(customConfig)}
{
}

void CustomShowScrapeJob::execute()
{
    // Because the custom TV scraper always starts with TMDb, the query should stay the same but
    // we have to correctly set the details that we want to load from TmdbTv.
    ShowScrapeJob::Config tmdbConfig = configFor(TmdbTv::ID, config().identifier);

    auto* tmdbJob = m_customConfig.tmdbTv->loadShow(tmdbConfig);
    connect(tmdbJob, &TmdbTvShowScrapeJob::sigFinished, this, &CustomShowScrapeJob::onTmdbLoaded);
    tmdbJob->execute();
}

void CustomShowScrapeJob::onTmdbLoaded(ShowScrapeJob* job)
{
    copyDetailsToShow(tvShow(), job->tvShow(), job->config().details);
    job->deleteLater();

    const QStringList scrapersToUse = m_customConfig.scraperForShowDetails.values();
    const bool loadImdb = tvShow().imdbId().isValid() && scrapersToUse.contains(ImdbTv::ID);
    const bool loadTvDb = tvShow().tvdbId().isValid() && scrapersToUse.contains(TheTvDb::ID);

    m_loadCounter = 1;

    if (loadImdb) {
        ++m_loadCounter;
    }
    if (loadTvDb) {
        ++m_loadCounter;
    }

    if (loadImdb) {
        loadWithScraper(ImdbTv::ID, ShowIdentifier(tvShow().imdbId()));
    }

    if (loadTvDb) {
        loadWithScraper(TheTvDb::ID, ShowIdentifier(tvShow().tvdbId()));
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
        qCritical() << "[CustomShowScrapeJob] Invalid scraper ID for custom tv scraper:" << scraperId;
        decreaseCounterAndCheckIfFinished();
        return;
    }

    auto* scrapeJob = scraper->loadShow(scraperConfig);
    connect(scrapeJob, &ShowScrapeJob::sigFinished, this, [this](ShowScrapeJob* job) {
        {
            // locking to avoid concurrent access to m_episodes
            QMutexLocker locker(&m_loadMutex);
            copyDetailsToShow(tvShow(), job->tvShow(), job->config().details);
        }
        job->deleteLater();
        decreaseCounterAndCheckIfFinished();
    });
    scrapeJob->execute();
}

void CustomShowScrapeJob::decreaseCounterAndCheckIfFinished()
{
    QMutexLocker locker(&m_loadMutex);
    --m_loadCounter;
    if (m_loadCounter <= 0) {
        locker.unlock();
        emit sigFinished(this);
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
    TvScraper* scraper = m_customConfig.scraperForId(scraperId);
    ScraperSettings* settings = Settings::instance()->scraperSettings(scraperId);

    if (scraper == nullptr) {
        qCritical() << "[CustomShowScrapeJob] Scraper not supported:" << scraperId;
        return mediaelch::Locale::English;
    }
    if (settings == nullptr) {
        return mediaelch::Locale::English;
    }

    return settings->language(scraper->meta().defaultLocale);
}

} // namespace scraper
} // namespace mediaelch
