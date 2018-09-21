#ifndef ALBUMCONTROLLER_H
#define ALBUMCONTROLLER_H

#include "globals/DownloadManagerElement.h"

#include <QObject>

class Album;
class DownloadManager;
class MediaCenterInterface;
class MusicScraperInterface;

class AlbumController : public QObject
{
    Q_OBJECT
public:
    explicit AlbumController(Album *parent = nullptr);
    ~AlbumController() override;

    bool saveData(MediaCenterInterface *mediaCenterInterface);
    bool loadData(MediaCenterInterface *mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(QString id, QString id2, MusicScraperInterface *scraperInterface, QList<MusicScraperInfos> infos);

    bool infoLoaded() const;
    void setInfoLoaded(bool infoLoaded);

    bool infoFromNfoLoaded() const;
    void setInfoFromNfoLoaded(bool infoFromNfoLoaded);

    bool downloadsInProgress() const;
    void abortDownloads();

    void loadImage(ImageType type, QUrl url);
    void loadImages(ImageType type, QList<QUrl> urls);
    void scraperLoadDone(MusicScraperInterface *scraper);

signals:
    void sigInfoLoadDone(Album *);
    void sigLoadingImages(Album *, QList<ImageType>);
    void sigLoadDone(Album *);
    void sigImage(Album *, ImageType, QByteArray);
    void sigLoadImagesStarted(Album *);
    void sigDownloadProgress(Album *, int, int);
    void sigSaved(Album *);

private slots:
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);
    void onFanartLoadDone(Album *album, QMap<ImageType, QList<Poster>> posters);

private:
    Album *m_album;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    DownloadManager *m_downloadManager;
    bool m_downloadsInProgress;
    int m_downloadsSize;
    int m_downloadsLeft;
    QList<MusicScraperInfos> m_infosToLoad;
};

#endif // ALBUMCONTROLLER_H
