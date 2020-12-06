#include "TvShowUpdater.h"

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "tv_shows/TvShow.h"
#include "ui/notifications/NotificationBox.h"

#ifndef EXTERN_QUAZIP
#    include "quazip/quazip/quazip.h"
#    include "quazip/quazip/quazipfile.h"
#else
#    include "quazip5/quazip.h"
#    include "quazip5/quazipfile.h"
#endif

#include <QBuffer>
#include <QDebug>

using namespace mediaelch;

TvShowUpdater::TvShowUpdater(QObject* parent) : QObject(parent), m_tvdb{nullptr}
{
    m_tvdb = dynamic_cast<scraper::TheTvDb*>(Manager::instance()->scrapers().tvScraper(scraper::TheTvDb::ID));
    if (m_tvdb == nullptr) {
        qCritical() << "[TvShowUpdater] Failing cast to TheTvDb scraper";
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
    if (!show->tvdbId().isValid() || (m_updatedShows.contains(show) && !force)) {
        return;
    }

    m_updatedShows.append(show);

    auto* box = NotificationBox::instance();
    const int value = box->value(Constants::TvShowUpdaterProgressMessageId);
    const int maxValue = box->maxValue(Constants::TvShowUpdaterProgressMessageId);
    box->progressBarProgress(value, maxValue + 1, Constants::TvShowUpdaterProgressMessageId);
    box->showProgressBar(tr("Updating TV Shows"), Constants::TvShowUpdaterProgressMessageId, true);

    Locale locale = Settings::instance()->scraperSettings(scraper::TheTvDb::ID)->language();
    scraper::ShowIdentifier id(show->tvdbId());
    scraper::SeasonScrapeJob::Config config{id, locale, {}, SeasonOrder::Aired, m_tvdb->meta().supportedEpisodeDetails};
    auto* scrapeJob = m_tvdb->loadSeasons(config);

    // Fill database with missing episodes.
    connect(scrapeJob, &scraper::SeasonScrapeJob::sigFinished, this, [show, box](scraper::SeasonScrapeJob* job) {
        job->deleteLater();
        box->hideProgressBar(Constants::TvShowUpdaterProgressMessageId);
        show->clearMissingEpisodes();

        // Store in database
        Database* const database = Manager::instance()->database();
        const int showsSettingsId = database->showsSettingsId(show);
        database->clearEpisodeList(showsSettingsId);
        for (auto* episode : job->episodes()) {
            database->addEpisodeToShowList(episode, showsSettingsId, episode->tvdbId());
        }
        database->cleanUpEpisodeList(showsSettingsId);

        show->fillMissingEpisodes();
    });
    scrapeJob->execute();
}
