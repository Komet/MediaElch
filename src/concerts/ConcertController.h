#pragma once

#include "data/TmdbId.h"
#include "globals/DownloadManagerElement.h"
#include "globals/Poster.h"
#include "globals/ScraperInfos.h"

#include <QMutex>
#include <QObject>

class Concert;
class DownloadManager;
class MediaCenterInterface;

namespace mediaelch {
namespace scraper {
class ConcertScraper;
}
} // namespace mediaelch

class ConcertController : public QObject
{
    Q_OBJECT
public:
    explicit ConcertController(Concert* parent = nullptr);

    Concert* concert();

    bool saveData(MediaCenterInterface* mediaCenterInterface);
    bool loadData(MediaCenterInterface* mediaCenterInterface, bool force = false, bool reloadFromNfo = true);
    void loadData(TmdbId id, mediaelch::scraper::ConcertScraper* scraperInterface, QSet<ConcertScraperInfo> infos);
    void loadStreamDetailsFromFile();
    void scraperLoadDone(mediaelch::scraper::ConcertScraper* scraper);
    QSet<ConcertScraperInfo> infosToLoad();
    bool infoLoaded() const;
    bool downloadsInProgress() const;
    void loadImage(ImageType type, QUrl url);
    void loadImages(ImageType type, QVector<QUrl> urls);
    void abortDownloads();
    void setLoadsLeft(QVector<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);
    void setInfosToLoad(QSet<ConcertScraperInfo> infos);

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
    Concert* m_concert = nullptr;
    bool m_infoLoaded = false;
    bool m_infoFromNfoLoaded = false;
    QSet<ConcertScraperInfo> m_infosToLoad;
    DownloadManager* m_downloadManager = nullptr;
    bool m_downloadsInProgress = false;
    int m_downloadsSize = 0;
    int m_downloadsLeft = 0;
    QVector<ScraperData> m_loadsLeft;
    bool m_loadDoneFired = false;
    QMutex m_loadMutex;
};
