#ifndef TVSCRAPERINTERFACE_H
#define TVSCRAPERINTERFACE_H

#include "data/MediaCenterInterface.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "Globals.h"

class TvShow;
class TvShowEpisode;

class TvScraperInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual void search(QString searchStr) = 0;
    virtual void loadTvShowData(QString id, TvShow *show) = 0;
    virtual void loadTvShowEpisodeData(QString id, TvShowEpisode *episode) = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings() = 0;
    virtual void saveSettings() = 0;
    virtual QWidget* settingsWidget() = 0;
signals:
    virtual void sigSearchDone(QList<ScraperSearchResult>) = 0;
};

#endif // TVSCRAPERINTERFACE_H
