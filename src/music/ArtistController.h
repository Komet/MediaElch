#pragma once

#include "globals/DownloadManagerElement.h"
#include "globals/Poster.h"
#include "globals/ScraperInfos.h"
#include "music/MusicBrainzId.h"

#include <QObject>

class Artist;
class DownloadManager;
class MediaCenterInterface;

namespace mediaelch {
namespace scraper {
class MusicScraper;
}
} // namespace mediaelch

class ArtistController : public QObject
{
    Q_OBJECT
public:
    explicit ArtistController(Artist* parent = nullptr);
    ~ArtistController() override = default;

    bool saveData(MediaCenterInterface* mediaCenterInterface);
    bool loadData(MediaCenterInterface* mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(MusicBrainzId id, mediaelch::scraper::MusicScraper* scraperInterface, QSet<MusicScraperInfo> infos);

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
    void sigInfoLoadDone(Artist*);
    void sigLoadingImages(Artist*, QVector<ImageType>);
    void sigLoadDone(Artist*);
    void sigImage(Artist*, ImageType, QByteArray);
    void sigLoadImagesStarted(Artist*);
    void sigDownloadProgress(Artist*, int, int);
    void sigSaved(Artist*);

private slots:
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);
    void onFanartLoadDone(Artist* artist, QMap<ImageType, QVector<Poster>> posters);

private:
    Artist* m_artist;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    DownloadManager* m_downloadManager;
    bool m_downloadsInProgress = false;
    int m_downloadsSize = 0;
    int m_downloadsLeft = 0;
    QSet<MusicScraperInfo> m_infosToLoad;
};
