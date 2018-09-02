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
    ~ArtistController() override;

    bool saveData(MediaCenterInterface *mediaCenterInterface);
    bool loadData(MediaCenterInterface *mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(QString id, MusicScraperInterface *scraperInterface, QList<MusicScraperInfos> infos);

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
    void sigInfoLoadDone(Artist *);
    void sigLoadingImages(Artist *, QList<ImageType>);
    void sigLoadDone(Artist *);
    void sigImage(Artist *, ImageType, QByteArray);
    void sigLoadImagesStarted(Artist *);
    void sigDownloadProgress(Artist *, int, int);
    void sigSaved(Artist *);

private slots:
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);
    void onFanartLoadDone(Artist *artist, QMap<ImageType, QList<Poster>> posters);

private:
    Artist *m_artist;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    DownloadManager *m_downloadManager;
    bool m_downloadsInProgress;
    int m_downloadsSize;
    int m_downloadsLeft;
    QList<MusicScraperInfos> m_infosToLoad;
};

#endif // ARTISTCONTROLLER_H
