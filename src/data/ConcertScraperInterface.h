#ifndef CONCERTSCRAPERINTERFACE_H
#define CONCERTSCRAPERINTERFACE_H

#include "data/Concert.h"
#include "globals/Globals.h"

#include <QSettings>

class Concert;
struct ScraperSearchResult;

/**
 * @brief The ConcertScraperInterface class
 * This class is the base for every concert Scraper.
 */
class ConcertScraperInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual void search(QString searchStr) = 0;
    virtual void loadData(TmdbId id, Concert *concert, QList<ConcertScraperInfos> infos) = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings(QSettings &settings) = 0;
    virtual void saveSettings(QSettings &settings) = 0;
    virtual QList<ConcertScraperInfos> scraperSupports() = 0;
    virtual QWidget *settingsWidget() = 0;
signals:
    virtual void searchDone(QList<ScraperSearchResult>) = 0;
};

#endif // CONCERTSCRAPERINTERFACE_H
