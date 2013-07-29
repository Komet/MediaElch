#ifndef SCRAPER_H
#define SCRAPER_H

#include <QSettings>
#include "data/MediaCenterInterface.h"
#include "movies/Movie.h"
#include "globals/Globals.h"

class Movie;
struct ScraperSearchResult;

/**
 * @brief The ScraperInterface class
 * This class is the base for every movie Scraper.
 */
class ScraperInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual QString identifier() = 0;
    virtual void search(QString searchStr) = 0;
    virtual void loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos) = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings(QSettings &settings) = 0;
    virtual void saveSettings(QSettings &settings) = 0;
    virtual QList<int> scraperSupports() = 0;
    virtual QList<int> scraperNativelySupports() = 0;
    virtual QWidget *settingsWidget() = 0;
signals:
    virtual void searchDone(QList<ScraperSearchResult>) = 0;
};

#endif // SCRAPER_H
