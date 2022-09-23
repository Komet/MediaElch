#pragma once

#include "data/TvDbId.h"
#include "globals/Globals.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/ScraperResult.h"
#include "settings/ScraperSettings.h"

#include <QString>
#include <QVector>

class TvShow;
class TvShowEpisode;

/// \brief The TvScraperInterface class
/// This class is the base for every TV show scraper.
class TvScraperInterface : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    virtual void search(QString searchStr) = 0;
    virtual void
    loadTvShowData(TvDbId id, TvShow* show, TvShowUpdateType updateType, QSet<ShowScraperInfo> infosToLoad) = 0;
    virtual void loadTvShowEpisodeData(TvDbId id, TvShowEpisode* episode, QSet<ShowScraperInfo> infosToLoad) = 0;
    virtual QWidget* settingsWidget() const = 0;

signals:
    void sigSearchDone(QVector<ScraperSearchResult>);
};
