#ifndef TVSCRAPERINTERFACE_H
#define TVSCRAPERINTERFACE_H

#include "globals/Globals.h"

#include <QList>
#include <QSettings>
#include <QString>

class TvShow;
class TvShowEpisode;

/**
 * @brief The TvScraperInterface class
 * This class is the base for every tv show scraper.
 */
class TvScraperInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual void search(QString searchStr) = 0;
    virtual void
    loadTvShowData(QString id, TvShow *show, TvShowUpdateType updateType, QList<TvShowScraperInfos> infosToLoad) = 0;
    virtual void loadTvShowEpisodeData(QString id, TvShowEpisode *episode, QList<TvShowScraperInfos> infosToLoad) = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings(QSettings &settings) = 0;
    virtual void saveSettings(QSettings &settings) = 0;
    virtual QWidget *settingsWidget() = 0;
    virtual QString identifier() = 0;
signals:
    virtual void sigSearchDone(QList<ScraperSearchResult>) = 0;
};

#endif // TVSCRAPERINTERFACE_H
