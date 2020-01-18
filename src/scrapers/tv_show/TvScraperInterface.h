#pragma once

#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperInterface.h"
#include "settings/ScraperSettings.h"
#include "tv_shows/TvDbId.h"

#include <QString>
#include <QVector>

class TvShow;
class TvShowEpisode;

/// @brief The TvScraperInterface class
/// This class is the base for every TV show scraper.
class TvScraperInterface : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    virtual void search(QString searchStr) = 0;
    virtual void
    loadTvShowData(TvDbId id, TvShow* show, TvShowUpdateType updateType, QVector<TvShowScraperInfos> infosToLoad) = 0;
    virtual void loadTvShowEpisodeData(TvDbId id, TvShowEpisode* episode, QVector<TvShowScraperInfos> infosToLoad) = 0;
    virtual QWidget* settingsWidget() const = 0;

signals:
    void sigSearchDone(QVector<ScraperSearchResult>);
};
