#pragma once

#include "globals/DownloadManagerElement.h"
#include "globals/Poster.h"
#include "globals/ScraperInfos.h"

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QVector>

class DownloadManager;
class MediaCenterInterface;
class Movie;
class MovieScraperInterface;

class MovieController : public QObject
{
    Q_OBJECT
public:
    explicit MovieController(Movie* parent = nullptr);

    bool saveData(MediaCenterInterface* mediaCenterInterface);
    bool loadData(MediaCenterInterface* mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(QMap<MovieScraperInterface*, QString> ids,
        MovieScraperInterface* scraperInterface,
        QVector<MovieScraperInfos> infos);
    void loadStreamDetailsFromFile();
    void scraperLoadDone(MovieScraperInterface* scraper);
    QVector<MovieScraperInfos> infosToLoad();
    bool infoLoaded() const;
    bool downloadsInProgress() const;
    void loadImage(ImageType type, QUrl url);
    void loadImages(ImageType type, QVector<QUrl> urls);
    void abortDownloads();
    void setLoadsLeft(QVector<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);
    void setInfosToLoad(QVector<MovieScraperInfos> infos);
    void setForceFanartBackdrop(const bool& force);
    void setForceFanartPoster(const bool& force);
    void setForceFanartCdArt(const bool& force);
    void setForceFanartClearArt(const bool& force);
    void setForceFanartLogo(const bool& force);

signals:
    void sigInfoLoadDone(Movie*);
    void sigLoadDone(Movie*);
    void sigLoadImagesStarted(Movie*);
    void sigDownloadProgress(Movie*, int, int);
    void sigLoadingImages(Movie*, QVector<ImageType>);
    void sigImage(Movie*, ImageType, QByteArray);

private slots:
    void onFanartLoadDone(Movie* movie, QMap<ImageType, QVector<Poster>> posters);
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);

private:
    Movie* m_movie;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    QVector<MovieScraperInfos> m_infosToLoad;
    DownloadManager* m_downloadManager;
    bool m_downloadsInProgress = false;
    int m_downloadsSize = 0;
    int m_downloadsLeft = 0;
    QVector<ScraperData> m_loadsLeft;
    bool m_loadDoneFired = 0;
    QMutex m_loadMutex;
    QMutex m_customScraperMutex;
    bool m_forceFanartBackdrop;
    bool m_forceFanartPoster;
    bool m_forceFanartClearArt;
    bool m_forceFanartCdArt;
    bool m_forceFanartLogo;
};
