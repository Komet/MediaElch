#ifndef SCRAPER_H
#define SCRAPER_H

#include "data/MediaCenterInterface.h"
#include "data/Movie.h"
#include "globals/Globals.h"

class Movie;

/**
 * @brief The ScraperInterface class
 * This class is the base for every movie Scraper.
 */
class ScraperInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual void search(QString searchStr) = 0;
    virtual void loadData(QString id, Movie *movie, QList<int> infos) = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings() = 0;
    virtual void saveSettings() = 0;
    virtual QWidget* settingsWidget() = 0;
    virtual QList<int> scraperSupports() = 0;
signals:
    virtual void searchDone(QList<ScraperSearchResult>) = 0;
};

#endif // SCRAPER_H
