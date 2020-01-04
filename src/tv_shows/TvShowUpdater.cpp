#include "TvShowUpdater.h"

#include <QBuffer>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"

#ifndef EXTERN_QUAZIP
#include "quazip/quazip/quazip.h"
#include "quazip/quazip/quazipfile.h"
#else
#include "quazip5/quazip.h"
#include "quazip5/quazipfile.h"
#endif

#include "scrapers/tv_show/TheTvDb.h"
#include "tv_shows/TvShow.h"
#include "ui/notifications/NotificationBox.h"

TvShowUpdater::TvShowUpdater(QObject* parent) : QObject(parent), m_tvdb{nullptr}
{
    for (TvScraperInterface* inter : Manager::instance()->tvScrapers()) {
        if (inter->identifier() == TheTvDb::scraperIdentifier) {
            m_tvdb = dynamic_cast<TheTvDb*>(inter);
            break;
        }
    }
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
    if (m_updatedShows.contains(show) && !force) {
        return;
    }

    m_updatedShows.append(show);

    auto* box = NotificationBox::instance();
    const int value = box->value(Constants::TvShowUpdaterProgressMessageId);
    const int maxValue = box->maxValue(Constants::TvShowUpdaterProgressMessageId);
    box->progressBarProgress(value, maxValue + 1, Constants::TvShowUpdaterProgressMessageId);
    box->showProgressBar(tr("Updating TV Shows"), Constants::TvShowUpdaterProgressMessageId, true);

    m_tvdb->fillDatabaseWithAllEpisodes(*show, [show, box]() {
        box->hideProgressBar(Constants::TvShowUpdaterProgressMessageId);
        show->clearMissingEpisodes();
        show->fillMissingEpisodes();
    });
}
