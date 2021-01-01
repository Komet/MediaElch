#pragma once

#include "globals/DownloadManagerElement.h"
#include "globals/Poster.h"
#include "globals/ScraperInfos.h"
#include "music/MusicBrainzId.h"

#include <QObject>

class Album;
class DownloadManager;
class MediaCenterInterface;

namespace mediaelch {
namespace scraper {
class MusicScraper;
}
} // namespace mediaelch

class AlbumController : public QObject
{
    Q_OBJECT
public:
    explicit AlbumController(Album* parent = nullptr);
    ~AlbumController() override;

    bool saveData(MediaCenterInterface* mediaCenterInterface);
    bool loadData(MediaCenterInterface* mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(MusicBrainzId id,
        MusicBrainzId id2,
        mediaelch::scraper::MusicScraper* scraperInterface,
        QSet<MusicScraperInfo> infos);

    bool infoLoaded() const;
    void setInfoLoaded(bool infoLoaded);

    bool infoFromNfoLoaded() const;
    void setInfoFromNfoLoaded(bool infoFromNfoLoaded);

    bool downloadsInProgress() const;
    void abortDownloads();

    void loadImage(ImageType type, QUrl url);
    void loadImages(ImageType type, QVector<QUrl> urls);
    void scraperLoadDone(mediaelch::scraper::MusicScraper* scraper);

signals:
    void sigInfoLoadDone(Album*);
    void sigLoadingImages(Album*, QVector<ImageType>);
    void sigLoadDone(Album*);
    void sigImage(Album*, ImageType, QByteArray);
    void sigLoadImagesStarted(Album*);
    void sigDownloadProgress(Album*, int, int);
    void sigSaved(Album*);

private slots:
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);
    void onFanartLoadDone(Album* album, QMap<ImageType, QVector<Poster>> posters);

private:
    Album* m_album;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    DownloadManager* m_downloadManager;
    bool m_downloadsInProgress = false;
    int m_downloadsSize = 0;
    int m_downloadsLeft = 0;
    QSet<MusicScraperInfo> m_infosToLoad;
};
