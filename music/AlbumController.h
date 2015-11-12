#ifndef ALBUMCONTROLLER_H
#define ALBUMCONTROLLER_H

#include <QObject>
#include "data/MediaCenterInterface.h"
#include "data/MusicScraperInterface.h"
#include "globals/DownloadManager.h"
#include "music/Album.h"

class Album;
class DownloadManager;
class MediaCenterInterface;

class AlbumController : public QObject
{
    Q_OBJECT
public:
    explicit AlbumController(Album *parent = 0);
    ~AlbumController();

    bool saveData(MediaCenterInterface *mediaCenterInterface);
    bool loadData(MediaCenterInterface *mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(QString id, QString id2, MusicScraperInterface *scraperInterface, QList<int> infos);

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
    void sigInfoLoadDone(Album*);
    void sigLoadingImages(Album*, QList<int>);
    void sigLoadDone(Album*);
    void sigImage(Album*,int,QByteArray);
    void sigLoadImagesStarted(Album*);
    void sigDownloadProgress(Album*, int, int);
    void sigSaved(Album*);

private slots:
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);
    void onFanartLoadDone(Album* album, QMap<int, QList<Poster> > posters);

private:
    Album *m_album;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    DownloadManager *m_downloadManager;
    bool m_downloadsInProgress;
    int m_downloadsSize;
    int m_downloadsLeft;
    QList<int> m_infosToLoad;
};

#endif // ALBUMCONTROLLER_H
