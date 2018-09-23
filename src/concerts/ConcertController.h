#ifndef CONCERTCONTROLLER_H
#define CONCERTCONTROLLER_H

#include "globals/DownloadManagerElement.h"

#include <QMutex>
#include <QObject>

class Concert;
class ConcertScraperInterface;
class DownloadManager;
class MediaCenterInterface;

class ConcertController : public QObject
{
    Q_OBJECT
public:
    explicit ConcertController(Concert *parent = nullptr);

    bool saveData(MediaCenterInterface *mediaCenterInterface);
    bool loadData(MediaCenterInterface *mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(QString id, ConcertScraperInterface *scraperInterface, QList<ConcertScraperInfos> infos);
    void loadStreamDetailsFromFile();
    void scraperLoadDone(ConcertScraperInterface *scraper);
    QList<ConcertScraperInfos> infosToLoad();
    bool infoLoaded() const;
    bool downloadsInProgress() const;
    void loadImage(ImageType type, QUrl url);
    void loadImages(ImageType type, QList<QUrl> urls);
    void abortDownloads();
    void setLoadsLeft(QList<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);
    void setInfosToLoad(QList<ConcertScraperInfos> infos);

signals:
    void sigInfoLoadDone(Concert *);
    void sigLoadDone(Concert *);
    void sigLoadImagesStarted(Concert *);
    void sigDownloadProgress(Concert *, int, int);
    void sigLoadingImages(Concert *, QList<ImageType>);
    void sigImage(Concert *, ImageType, QByteArray);

private slots:
    void onFanartLoadDone(Concert *concert, QMap<ImageType, QList<Poster>> posters);
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);

private:
    Concert *m_concert;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    QList<ConcertScraperInfos> m_infosToLoad;
    DownloadManager *m_downloadManager;
    bool m_downloadsInProgress;
    int m_downloadsSize;
    int m_downloadsLeft;
    QList<ScraperData> m_loadsLeft;
    bool m_loadDoneFired;
    QMutex m_loadMutex;
};

#endif // CONCERTCONTROLLER_H
