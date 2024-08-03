#include "scrapers/tv_show/custom/CustomSeasonScrapeJob.h"

#include "globals/Manager.h"
#include "log/Log.h"
#include "scrapers/tv_show/ShowMerger.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvSeasonScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"
#include "utils/Containers.h"

#include <QMutexLocker>
#include <utility>

namespace mediaelch {
namespace scraper {

CustomSeasonScrapeJob::CustomSeasonScrapeJob(CustomTvScraperConfiguration& customConfig,
    SeasonScrapeJob::Config config,
    QObject* parent) :
    SeasonScrapeJob(config, parent), m_customConfig{customConfig}
{
}

void CustomSeasonScrapeJob::doStart()
{
    // Because the custom TV scraper always starts with TMDB, we have to load the show identifiers
    // from TMDB before starting to load episodes.
    // Only load basic details, i.e. the title (which includes IDs).
    ShowScrapeJob::Config tmdbConfig;
    tmdbConfig.identifier = config().showIdentifier;
    tmdbConfig.details = {ShowScraperInfo::Title};

    auto* tmdbJob = m_customConfig.tmdbTv->loadShow(tmdbConfig);
    connect(tmdbJob, &TmdbTvShowScrapeJob::loadFinished, this, &CustomSeasonScrapeJob::onTmdbShowLoaded);
    tmdbJob->start();
}

void CustomSeasonScrapeJob::onTmdbShowLoaded(ShowScrapeJob* job)
{
    const QStringList showScrapersToUse = m_customConfig.scraperForShowDetails.values();
    const QStringList episodeScrapersToUse = m_customConfig.scraperForEpisodeDetails.values();

    const bool loadTmdb = job->tvShow().tmdbId().isValid()
                          && (showScrapersToUse.contains(TmdbTv::ID) || episodeScrapersToUse.contains(TmdbTv::ID));
    const bool loadImdb = job->tvShow().imdbId().isValid()
                          && (showScrapersToUse.contains(ImdbTv::ID) || episodeScrapersToUse.contains(ImdbTv::ID));

    if (loadTmdb) {
        ++m_loadCounter;
    }
    if (loadImdb) {
        ++m_loadCounter;
    }

    if (loadTmdb) {
        loadWithScraper(TmdbTv::ID, config().showIdentifier);
    }
    if (loadImdb) {
        loadWithScraper(ImdbTv::ID, ShowIdentifier(job->tvShow().imdbId()));
    }

    job->deleteLater();
}

void CustomSeasonScrapeJob::loadWithScraper(const QString& scraperId, const ShowIdentifier& identifier)
{
    SeasonScrapeJob::Config scraperConfig = configFor(scraperId, identifier);
    if (scraperConfig.details.isEmpty()) {
        // No need to load from scraper if no details are requested.
        decreaseCounterAndCheckIfFinished();
        return;
    }

    TvScraper* scraper = m_customConfig.scraperForId(scraperId);
    if (scraper == nullptr) {
        qCCritical(generic) << "[CustomSeasonScrapeJob] Invalid scraper ID for custom tv scraper:" << scraperId;
        decreaseCounterAndCheckIfFinished();
        return;
    }

    auto* scrapeJob = scraper->loadSeasons(scraperConfig);
    connect(scrapeJob, &SeasonScrapeJob::loadFinished, this, [this](SeasonScrapeJob* job) {
        {
            // locking to avoid concurrent access to m_episodes
            QMutexLocker locker(&m_loadMutex);
            copyDetailsToEpisodeMap(m_episodes, job->episodes(), job->config().details, this);
        }
        job->deleteLater();
        decreaseCounterAndCheckIfFinished();
    });
    scrapeJob->start();
}

void CustomSeasonScrapeJob::decreaseCounterAndCheckIfFinished()
{
    QMutexLocker locker(&m_loadMutex);
    --m_loadCounter;
    if (m_loadCounter <= 0) {
        locker.unlock();
        emitFinished();
    }
}

SeasonScrapeJob::Config CustomSeasonScrapeJob::configFor(const QString& scraperId, const ShowIdentifier& id)
{
    SeasonScrapeJob::Config scraperConfig = config();
    scraperConfig.locale = localeFor(scraperId);
    scraperConfig.showIdentifier = id;

    auto detailsForScraper = mediaelch::listToSet(m_customConfig.scraperForEpisodeDetails.keys(scraperId));
    detailsForScraper.intersect(scraperConfig.details);
    scraperConfig.details = detailsForScraper;

    return scraperConfig;
}

Locale CustomSeasonScrapeJob::localeFor(const QString& scraperId) const
{
    return Settings::instance()->value({"scrapers", QStringLiteral("Scrapers/%1/Language").arg(scraperId)}).toString();
}

} // namespace scraper
} // namespace mediaelch
