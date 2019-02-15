#pragma once

#include "data/TmdbId.h"
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
    explicit ConcertController(Concert* parent = nullptr);

    Concert* concert();

    bool saveData(MediaCenterInterface* mediaCenterInterface);
    bool loadData(MediaCenterInterface* mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(TmdbId id, ConcertScraperInterface* scraperInterface, QVector<ConcertScraperInfos> infos);
    void loadStreamDetailsFromFile();
    void scraperLoadDone(ConcertScraperInterface* scraper);
    QVector<ConcertScraperInfos> infosToLoad();
    bool infoLoaded() const;
    bool downloadsInProgress() const;
    void loadImage(ImageType type, QUrl url);
    void loadImages(ImageType type, QVector<QUrl> urls);
    void abortDownloads();
    void setLoadsLeft(QVector<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);
    void setInfosToLoad(QVector<ConcertScraperInfos> infos);

signals:
    void sigInfoLoadDone(Concert*);
    void sigLoadDone(Concert*);
    void sigLoadImagesStarted(Concert*);
    void sigDownloadProgress(Concert*, int, int);
    void sigLoadingImages(Concert*, QVector<ImageType>);
    void sigImage(Concert*, ImageType, QByteArray);

private slots:
    void onFanartLoadDone(Concert* concert, QMap<ImageType, QVector<Poster>> posters);
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);

private:
    Concert* m_concert;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    QVector<ConcertScraperInfos> m_infosToLoad;
    DownloadManager* m_downloadManager;
    bool m_downloadsInProgress;
    int m_downloadsSize;
    int m_downloadsLeft;
    QVector<ScraperData> m_loadsLeft;
    bool m_loadDoneFired;
    QMutex m_loadMutex;
};
