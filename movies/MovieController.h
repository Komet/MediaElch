#ifndef MOVIECONTROLLER_H
#define MOVIECONTROLLER_H

#include <QMutex>
#include <QObject>

#include "data/MediaCenterInterface.h"
#include "data/ScraperInterface.h"
#include "globals/DownloadManager.h"
#include "globals/DownloadManagerElement.h"
#include "data/Movie.h"

class DownloadManager;
class DownloadManagerElement;
class MediaCenterInterface;
class Movie;
class ScraperInterface;

class MovieController : public QObject
{
    Q_OBJECT
public:
    explicit MovieController(Movie *parent = nullptr);

    bool saveData(MediaCenterInterface *mediaCenterInterface);
    bool loadData(MediaCenterInterface *mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void
    loadData(QMap<ScraperInterface *, QString> ids, ScraperInterface *scraperInterface, QList<MovieScraperInfos> infos);
    void loadStreamDetailsFromFile();
    void scraperLoadDone(ScraperInterface *scraper);
    QList<MovieScraperInfos> infosToLoad();
    bool infoLoaded() const;
    bool downloadsInProgress() const;
    void loadImage(ImageType type, QUrl url);
    void loadImages(ImageType type, QList<QUrl> urls);
    void abortDownloads();
    void setLoadsLeft(QList<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);
    void setInfosToLoad(QList<MovieScraperInfos> infos);
    void setForceFanartBackdrop(const bool &force);
    void setForceFanartPoster(const bool &force);
    void setForceFanartCdArt(const bool &force);
    void setForceFanartClearArt(const bool &force);
    void setForceFanartLogo(const bool &force);

signals:
    void sigInfoLoadDone(Movie *);
    void sigLoadDone(Movie *);
    void sigLoadImagesStarted(Movie *);
    void sigDownloadProgress(Movie *, int, int);
    void sigLoadingImages(Movie *, QList<ImageType>);
    void sigImage(Movie *, ImageType, QByteArray);

private slots:
    void onFanartLoadDone(Movie *movie, QMap<ImageType, QList<Poster>> posters);
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);

private:
    Movie *m_movie;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    QList<MovieScraperInfos> m_infosToLoad;
    DownloadManager *m_downloadManager;
    bool m_downloadsInProgress;
    int m_downloadsSize;
    int m_downloadsLeft;
    QList<ScraperData> m_loadsLeft;
    bool m_loadDoneFired;
    QMutex m_loadMutex;
    QMutex m_customScraperMutex;
    bool m_forceFanartBackdrop;
    bool m_forceFanartPoster;
    bool m_forceFanartClearArt;
    bool m_forceFanartCdArt;
    bool m_forceFanartLogo;
};

#endif // MOVIECONTROLLER_H
