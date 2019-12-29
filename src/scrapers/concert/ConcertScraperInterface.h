#pragma once

#include "concerts/Concert.h"
#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperInterface.h"
#include "settings/ScraperSettings.h"

class Concert;

/// @brief The ConcertScraperInterface class
/// This class is the base for every concert Scraper.
class ConcertScraperInterface : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    ConcertScraperInterface() : QObject() {}
    virtual void search(QString searchStr) = 0;
    virtual void loadData(TmdbId id, Concert* concert, QVector<ConcertScraperInfos> infos) = 0;
    virtual QVector<ConcertScraperInfos> scraperSupports() = 0;
    virtual QWidget* settingsWidget() = 0;

signals:
    void searchDone(QVector<ScraperSearchResult>);
};
