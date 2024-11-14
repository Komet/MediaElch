#include "TvShowUpdater.h"

#include "data/tv_show/TvShow.h"
#include "database/TvShowPersistence.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "log/Log.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "ui/notifications/NotificationBox.h"

#include <QBuffer>

using namespace mediaelch;

TvShowUpdater::TvShowUpdater(QObject* parent) : QObject(parent), m_tmdb{nullptr}
{
    m_tmdb = dynamic_cast<scraper::TmdbTv*>(Manager::instance()->scrapers().tvScraper(scraper::TmdbTv::ID));
    if (m_tmdb == nullptr) {
        qCCritical(generic) << "[TvShowUpdater] Failing cast to TmdbTv scraper";
    }
}

TvShowUpdater* TvShowUpdater::instance(QObject* parent)
{
    static auto* instance = new TvShowUpdater(parent);
    return instance;
}

void TvShowUpdater::updateShow(TvShow* show, bool force)
{
    using namespace mediaelch;
    if (!show->tmdbId().isValid() || (m_updatedShows.contains(show) && !force)) {
        return;
    }

    m_updatedShows.append(show);

    auto* box = NotificationBox::instance();
    const int value = box->value(Constants::TvShowUpdaterProgressMessageId);
    const int maxValue = box->maxValue(Constants::TvShowUpdaterProgressMessageId);
    box->progressBarProgress(value, maxValue + 1, Constants::TvShowUpdaterProgressMessageId);
    box->showProgressBar(tr("Updating TV Shows"), Constants::TvShowUpdaterProgressMessageId, true);

    // TODO: Can we use TMDB's settings?
    Locale locale = Settings::instance()->value({"scrapers", "Scrapers/tmdbtv/Language"}).toString();
    scraper::ShowIdentifier id(show->tmdbId());
    scraper::SeasonScrapeJob::Config config{id, locale, {}, SeasonOrder::Aired, m_tmdb->meta().supportedEpisodeDetails};
    auto* scrapeJob = m_tmdb->loadSeasons(config);

    // Fill database with missing episodes.
    connect(scrapeJob, &scraper::SeasonScrapeJob::loadFinished, this, [show, box](scraper::SeasonScrapeJob* job) {
        job->deleteLater();
        box->hideProgressBar(Constants::TvShowUpdaterProgressMessageId);
        show->clearMissingEpisodes();

        const auto& scrapedEpisodes = job->episodes();

        for (TvShowEpisode* episode : scrapedEpisodes) {
            // Map according to advanced settings
            const QStringList networks = helper::mapStudio(episode->networks());
            const Certification certification = helper::mapCertification(episode->certification());

            episode->setNetworks(networks);
            episode->setCertification(certification);
        }

        // Store in database
        TvShowPersistence database{*Manager::instance()->database()};
        const DatabaseId showsSettingsId = database.showsSettingsId(show);
        database.clearEpisodeList(showsSettingsId);
        for (auto* episode : scrapedEpisodes) {
            database.addEpisodeToShowList(episode, showsSettingsId, episode->tmdbId());
        }
        database.cleanUpEpisodeList(showsSettingsId);

        show->fillMissingEpisodes();
    });
    scrapeJob->start();
}
