#pragma once

#include "globals/DownloadManagerElement.h"
#include "globals/Poster.h"
#include "globals/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QVector>

class DownloadManager;
class MediaCenterInterface;
class Movie;

namespace mediaelch {
namespace scraper {
class MovieScraper;
}
} // namespace mediaelch

class MovieController : public QObject
{
    Q_OBJECT
public:
    explicit MovieController(Movie* parent = nullptr);

    /// \brief Saves the movies infos with the given MediaCenterInterface
    /// \param mediaCenterInterface MediaCenterInterface to use for saving
    /// \return Saving was successful or not
    bool saveData(MediaCenterInterface* mediaCenterInterface);

    /// \brief Loads the movies infos with the given MediaCenter
    /// \param mediaCenterInterface MediaCenterInterface to use for loading
    /// \param force Force the loading. If set to false and infos were already loeaded this function just returns
    /// \return Loading was successful or not
    bool loadData(MediaCenterInterface* mediaCenterInterface, bool force = false, bool reloadFromNfo = true);

    void loadStreamDetailsFromFile();

    /// \brief Loads images that were requested but not available through the scraper from Fanart.tv
    void loadMissingImagesFromFanartTv(mediaelch::scraper::MovieScraper& scraper);

    QVector<MovieScraperInfos> infosToLoad();

    /// \brief Holds wether movie infos were loaded from a MediaCenterInterface or ScraperInterface
    /// \return Infos were loaded
    bool infoLoaded() const;

    /// \brief Returns true if a download is in progress
    /// \return Download is in progress
    bool downloadsInProgress() const;

    void loadImage(ImageType type, QUrl url);
    void loadImages(ImageType type, QVector<QUrl> urls);
    void abortDownloads();
    void setLoadsLeft(QVector<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);
    void setInfosToLoad(QVector<MovieScraperInfos> infos);

signals:
    void sigLoadStarted(Movie*);
    void sigInfoLoadDone(Movie*);
    void sigLoadDone(Movie*);
    void sigLoadImagesStarted(Movie*);
    void sigDownloadProgress(Movie*, int, int);
    void sigLoadingImages(Movie*, QVector<ImageType>);
    void sigImage(Movie*, ImageType, QByteArray);

public slots:
    void scraperLoadSuccess();
    void scraperLoadError(ScraperLoadError error);

private slots:
    void onFanartLoadDone(Movie* movie, QMap<ImageType, QVector<Poster>> posters);
    void onAllDownloadsFinished();
    void onDownloadFinished(DownloadManagerElement elem);

private:
    Movie* m_movie;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    bool m_loadMissingImagesFromFanartTv = false;
    QVector<MovieScraperInfos> m_infosToLoad;
    DownloadManager* m_downloadManager;
    bool m_downloadsInProgress = false;
    bool m_scrapeInProgress = false;
    int m_downloadsSize = 0;
    int m_downloadsLeft = 0;
    QVector<ScraperData> m_loadsLeft;
    bool m_loadDoneFired = 0;
    QMutex m_loadMutex;
    QMutex m_customScraperMutex;
};
