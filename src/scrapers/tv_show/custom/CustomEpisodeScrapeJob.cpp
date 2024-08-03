#include "scrapers/tv_show/custom/CustomEpisodeScrapeJob.h"

#include "globals/Manager.h"
#include "log/Log.h"
#include "scrapers/tv_show/ShowMerger.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.h"
#include "utils/Containers.h"

#include <QMutexLocker>
#include <utility>

namespace mediaelch {
namespace scraper {

CustomEpisodeScrapeJob::CustomEpisodeScrapeJob(CustomTvScraperConfiguration& customConfig,
    EpisodeScrapeJob::Config config,
    QObject* parent) :
    EpisodeScrapeJob(config, parent), m_customConfig{customConfig}
{
}

void CustomEpisodeScrapeJob::doStart()
{
    // Because the custom TV scraper always starts with TMDB, the query should stay the same but
    // we have to correctly set the details that we want to load from TmdbTv.
    EpisodeScrapeJob::Config tmdbConfig = configFor(TmdbTv::ID, config().identifier);

    if (tmdbConfig.details.isEmpty()) {
        // HACK: in onTmdbLoaded() we copy details to this job's show.
        //       But if we do not load any details from TMDB, we don't copy anything
        //       not even the IDs that are needed for other scrapers, etc.
        //       By using this hack, we always invoke copyDetailsToShow() so that IDs are copied.
        tmdbConfig.details.insert(EpisodeScraperInfo::Invalid);
    }

    auto* tmdbJob = m_customConfig.tmdbTv->loadEpisode(tmdbConfig);
    connect(tmdbJob, &TmdbTvEpisodeScrapeJob::loadFinished, this, &CustomEpisodeScrapeJob::onTmdbLoaded);
    tmdbJob->start();
}

void CustomEpisodeScrapeJob::onTmdbLoaded(EpisodeScrapeJob* job)
{
    copyDetailsToEpisode(episode(), job->episode(), job->config().details);
    job->deleteLater();

    const QStringList scrapersToUse = m_customConfig.scraperForEpisodeDetails.values();
    const bool loadImdb = episode().imdbId().isValid() && scrapersToUse.contains(ImdbTv::ID);

    m_loadCounter = 1;

    if (loadImdb) {
        ++m_loadCounter;
    }

    if (loadImdb) {
        loadWithScraper(ImdbTv::ID, EpisodeIdentifier(episode().imdbId()));
    }

    decreaseCounterAndCheckIfFinished();
}

void CustomEpisodeScrapeJob::loadWithScraper(const QString& scraperId, const EpisodeIdentifier& identifier)
{
    EpisodeScrapeJob::Config scraperConfig = configFor(scraperId, identifier);
    if (scraperConfig.details.isEmpty()) {
        // No need to load from scraper if no details are requested.
        decreaseCounterAndCheckIfFinished();
        return;
    }

    TvScraper* scraper = m_customConfig.scraperForId(scraperId);
    if (scraper == nullptr) {
        qCCritical(generic) << "[CustomEpisodeScrapeJob] Invalid scraper ID for custom tv scraper:" << scraperId;
        decreaseCounterAndCheckIfFinished();
        return;
    }

    auto* scrapeJob = scraper->loadEpisode(scraperConfig);
    connect(scrapeJob, &EpisodeScrapeJob::loadFinished, this, [this](EpisodeScrapeJob* job) {
        copyDetailsToEpisode(episode(), job->episode(), job->config().details);
        job->deleteLater();
        decreaseCounterAndCheckIfFinished();
    });
    scrapeJob->start();
}

void CustomEpisodeScrapeJob::decreaseCounterAndCheckIfFinished()
{
    --m_loadCounter;
    if (m_loadCounter <= 0) {
        emitFinished();
    }
}

EpisodeScrapeJob::Config CustomEpisodeScrapeJob::configFor(const QString& scraperId, const EpisodeIdentifier& id)
{
    EpisodeScrapeJob::Config scraperConfig = config();
    scraperConfig.locale = localeFor(scraperId);
    scraperConfig.identifier = id;

    auto detailsForScraper = mediaelch::listToSet(m_customConfig.scraperForEpisodeDetails.keys(scraperId));
    detailsForScraper.intersect(scraperConfig.details);
    scraperConfig.details = detailsForScraper;

    return scraperConfig;
}

Locale CustomEpisodeScrapeJob::localeFor(const QString& scraperId) const
{
    return Settings::instance()->value({"scrapers", QStringLiteral("Scrapers/%1/Language").arg(scraperId)}).toString();
}

} // namespace scraper
} // namespace mediaelch
