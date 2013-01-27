#ifndef MOVIECONTROLLER_H
#define MOVIECONTROLLER_H

#include <QMutex>
#include <QObject>
#include "data/MediaCenterInterface.h"
#include "data/ScraperInterface.h"
#include "movies/Movie.h"
#include "globals/DownloadManagerElement.h"
#include "globals/DownloadManager.h"

class DownloadManager;
class DownloadManagerElement;
class MediaCenterInterface;
class Movie;
class ScraperInterface;

class MovieController : public QObject
{
    Q_OBJECT
public:
    explicit MovieController(Movie *parent = 0);

    bool saveData(MediaCenterInterface *mediaCenterInterface);
    bool loadData(MediaCenterInterface *mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(QString id, ScraperInterface *scraperInterface, QList<int> infos);
    void loadStreamDetailsFromFile();
    void scraperLoadDone();
    QList<int> infosToLoad();
    bool infoLoaded() const;
    bool downloadsInProgress() const;
    int downloadsSize() const;
    void loadImage(int type, QUrl url);
    void loadImages(int type, QList<QUrl> urls);
    void abortDownloads();
    void setLoadsLeft(QList<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);
    void setInfosToLoad(QList<int> infos);

signals:
    void sigInfoLoadDone(Movie*);
    void sigLoadDone(Movie*);
    void sigLoadImagesStarted(Movie*);
    void sigDownloadProgress(Movie*, int, int);
    void sigLoadingImages(Movie*, QList<int>);
    void sigImage(Movie*,int,QImage);

private slots:
    void onFanartLoadDone(Movie* movie, QMap<int, QList<Poster> > posters);
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);

private:
    Movie *m_movie;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    QList<int> m_infosToLoad;
    DownloadManager *m_downloadManager;
    bool m_downloadsInProgress;
    int m_downloadsSize;
    int m_downloadsLeft;
    QList<ScraperData> m_loadsLeft;
    bool m_loadDoneFired;
    QMutex m_loadMutex;
};

#endif // MOVIECONTROLLER_H
