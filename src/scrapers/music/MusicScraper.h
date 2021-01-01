#pragma once

#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "globals/ScraperResult.h"
#include "music/MusicBrainzId.h"
#include "scrapers/ScraperInterface.h"
#include "settings/ScraperSettings.h"

#include <QString>
#include <QVector>
#include <QWidget>

class Album;
class Artist;

namespace mediaelch {
namespace scraper {

class MusicScraper : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    virtual QString name() const = 0;
    virtual QString identifier() const = 0;
    virtual void searchAlbum(QString artistName, QString searchStr) = 0;
    virtual void searchArtist(QString searchStr) = 0;
    virtual void loadData(MusicBrainzId id, Artist* artist, QSet<MusicScraperInfo> infos) = 0;
    virtual void loadData(MusicBrainzId id, MusicBrainzId id2, Album* album, QSet<MusicScraperInfo> infos) = 0;
    virtual QSet<MusicScraperInfo> scraperSupports() = 0;
    virtual QWidget* settingsWidget() = 0;

signals:
    void sigSearchDone(QVector<ScraperSearchResult>);
};

} // namespace scraper
} // namespace mediaelch
