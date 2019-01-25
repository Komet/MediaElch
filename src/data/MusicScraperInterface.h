#pragma once

#include "data/ScraperInterface.h"
#include "globals/Globals.h"
#include "settings/ScraperSettings.h"

#include <QList>
#include <QString>
#include <QWidget>

class Album;
class Artist;
struct ScraperSearchResult;

class MusicScraperInterface : public ScraperInterface, public QObject
{
public:
    virtual void searchAlbum(QString artistName, QString searchStr) = 0;
    virtual void searchArtist(QString searchStr) = 0;
    virtual void loadData(QString id, Artist *artist, QList<MusicScraperInfos> infos) = 0;
    virtual void loadData(QString id, QString id2, Album *album, QList<MusicScraperInfos> infos) = 0;
    virtual QList<MusicScraperInfos> scraperSupports() = 0;
    virtual QWidget *settingsWidget() = 0;

signals:
    virtual void sigSearchDone(QList<ScraperSearchResult>) = 0;
};
