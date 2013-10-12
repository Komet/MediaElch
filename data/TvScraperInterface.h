#ifndef TVSCRAPERINTERFACE_H
#define TVSCRAPERINTERFACE_H

#include <QSettings>
#include "data/MediaCenterInterface.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "globals/Globals.h"

class TvShow;
class TvShowEpisode;
struct ScraperSearchResult;

/**
 * @brief The TvScraperInterface class
 * This class is the base for every tv show scraper.
 */
class TvScraperInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual void search(QString searchStr) = 0;
    virtual void loadTvShowData(QString id, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad) = 0;
    virtual void loadTvShowEpisodeData(QString id, TvShowEpisode *episode, QList<int> infosToLoad) = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings(QSettings &settings) = 0;
    virtual void saveSettings(QSettings &settings) = 0;
    virtual QWidget *settingsWidget() = 0;
    virtual QString identifier() = 0;
signals:
    virtual void sigSearchDone(QList<ScraperSearchResult>) = 0;
};

#endif // TVSCRAPERINTERFACE_H
