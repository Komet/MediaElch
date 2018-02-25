#ifndef ARTISTCONTROLLER_H
#define ARTISTCONTROLLER_H

#include <QObject>

#include "data/MediaCenterInterface.h"
#include "data/MusicScraperInterface.h"
#include "globals/DownloadManager.h"
#include "music/Artist.h"

class Artist;
class DownloadManager;
class MediaCenterInterface;
class MusicScraperInterface;

class ArtistController : public QObject
{
    Q_OBJECT
public:
    explicit ArtistController(Artist *parent = nullptr);
    ~ArtistController();

    bool saveData(MediaCenterInterface *mediaCenterInterface);
    bool loadData(MediaCenterInterface *mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(QString id, MusicScraperInterface *scraperInterface, QList<int> infos);

    bool infoLoaded() const;
    void setInfoLoaded(bool infoLoaded);

    bool infoFromNfoLoaded() const;
    void setInfoFromNfoLoaded(bool infoFromNfoLoaded);

    bool downloadsInProgress() const;
    void abortDownloads();

    void loadImage(int type, QUrl url);
    void loadImages(int type, QList<QUrl> urls);
    void scraperLoadDone(MusicScraperInterface *scraper);

signals:
    void sigInfoLoadDone(Artist*);
    void sigLoadingImages(Artist*, QList<int>);
    void sigLoadDone(Artist*);
    void sigImage(Artist*,int,QByteArray);
    void sigLoadImagesStarted(Artist*);
    void sigDownloadProgress(Artist*, int, int);
    void sigSaved(Artist*);

private slots:
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);
    void onFanartLoadDone(Artist* artist, QMap<int, QList<Poster> > posters);

private:
    Artist *m_artist;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    DownloadManager *m_downloadManager;
    bool m_downloadsInProgress;
    int m_downloadsSize;
    int m_downloadsLeft;
    QList<int> m_infosToLoad;
};

#endif // ARTISTCONTROLLER_H
