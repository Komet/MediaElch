#pragma once

#include "data/ScraperInterface.h"
#include "data/TvDbId.h"
#include "globals/Globals.h"
#include "settings/ScraperSettings.h"

#include <QString>
#include <QVector>

class TvShow;
class TvShowEpisode;

/**
 * @brief The TvScraperInterface class
 * This class is the base for every tv show scraper.
 */
class TvScraperInterface : public ScraperInterface, public QObject
{
public:
    virtual void search(QString searchStr) = 0;
    virtual void
    loadTvShowData(TvDbId id, TvShow* show, TvShowUpdateType updateType, QVector<TvShowScraperInfos> infosToLoad) = 0;
    virtual void loadTvShowEpisodeData(TvDbId id, TvShowEpisode* episode, QVector<TvShowScraperInfos> infosToLoad) = 0;
    virtual QWidget* settingsWidget() = 0;

signals:
    virtual void sigSearchDone(QVector<ScraperSearchResult>) = 0;
};
