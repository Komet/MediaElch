#pragma once

#include "data/Concert.h"
#include "data/ScraperInterface.h"
#include "globals/Globals.h"
#include "settings/ScraperSettings.h"

class Concert;
struct ScraperSearchResult;

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
    virtual void searchDone(QVector<ScraperSearchResult>) = 0;
};
