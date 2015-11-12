#ifndef MUSICSCRAPERINTERFACE
#define MUSICSCRAPERINTERFACE

#include <QSettings>
#include "../data/MediaCenterInterface.h"
#include "../globals/Globals.h"
#include "../music/Album.h"
#include "../music/Artist.h"

class Album;
class Artist;
struct ScraperSearchResult;

class MusicScraperInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual QString identifier() = 0;
    virtual void searchAlbum(QString artistName, QString searchStr) = 0;
    virtual void searchArtist(QString searchStr) = 0;
    virtual void loadData(QString id, Artist *artist, QList<int> infos) = 0;
    virtual void loadData(QString id, QString id2, Album *album, QList<int> infos) = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings(QSettings &settings) = 0;
    virtual void saveSettings(QSettings &settings) = 0;
    virtual QList<int> scraperSupports() = 0;
    virtual QWidget *settingsWidget() = 0;
signals:
    virtual void sigSearchDone(QList<ScraperSearchResult>) = 0;
};

#endif // MUSICSCRAPERINTERFACE

