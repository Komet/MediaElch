#pragma once

#include "data/ScraperInterface.h"
#include "globals/Globals.h"
#include "settings/ScraperSettings.h"

#include <QString>
#include <QVector>
#include <QWidget>

class Album;
class Artist;
struct ScraperSearchResult;

class MusicScraperInterface : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    virtual void searchAlbum(QString artistName, QString searchStr) = 0;
    virtual void searchArtist(QString searchStr) = 0;
    virtual void loadData(QString id, Artist* artist, QVector<MusicScraperInfos> infos) = 0;
    virtual void loadData(QString id, QString id2, Album* album, QVector<MusicScraperInfos> infos) = 0;
    virtual QVector<MusicScraperInfos> scraperSupports() = 0;
    virtual QWidget* settingsWidget() = 0;

signals:
    virtual void sigSearchDone(QVector<ScraperSearchResult>) = 0;
};
