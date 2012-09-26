#ifndef CONCERTSCRAPERINTERFACE_H
#define CONCERTSCRAPERINTERFACE_H

#include "data/MediaCenterInterface.h"
#include "data/Concert.h"
#include "globals/Globals.h"

class Concert;

/**
 * @brief The ConcertScraperInterface class
 * This class is the base for every concert Scraper.
 */
class ConcertScraperInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual void search(QString searchStr) = 0;
    virtual void loadData(QString id, Concert *concert, QList<int> infos) = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings() = 0;
    virtual void saveSettings() = 0;
    virtual QList<int> scraperSupports() = 0;
    virtual QMap<QString, QString> languages() = 0;
    virtual QString language() = 0;
    virtual void setLanguage(QString language) = 0;
signals:
    virtual void searchDone(QList<ScraperSearchResult>) = 0;
};

#endif // CONCERTSCRAPERINTERFACE_H
