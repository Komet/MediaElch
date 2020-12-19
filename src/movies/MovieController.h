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

    /// \brief Loads the movies infos with the given MediaCenterInterface
    /// \param mediaCenterInterface MediaCenterInterface to use for loading
    /// \param force Force the loading. If set to false and infos were already loeaded this function just returns
    /// \return Loading was successful or not
    bool loadData(MediaCenterInterface* mediaCenterInterface, bool force = false, bool reloadFromNfo = true);

    /// \brief Loads the movies info from a scraper
    /// \param ids Id of the movie within the given ScraperInterface
    /// \param scraperInterface ScraperInterface to use for loading
    /// \param infos List of infos to load
    void loadData(QHash<mediaelch::scraper::MovieScraper*, QString> ids,
        mediaelch::scraper::MovieScraper* scraperInterface,
        QSet<MovieScraperInfo> infos);

    void loadStreamDetailsFromFile();

    /// \brief Called when a ScraperInterface has finished loading
    ///        Emits the loaded signal
    void scraperLoadDone(mediaelch::scraper::MovieScraper* scraper);

    QSet<MovieScraperInfo> infosToLoad();

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
    void setInfosToLoad(QSet<MovieScraperInfo> infos);
    void setForceFanartBackdrop(const bool& force);
    void setForceFanartPoster(const bool& force);
    void setForceFanartCdArt(const bool& force);
    void setForceFanartClearArt(const bool& force);
    void setForceFanartLogo(const bool& force);

signals:
    void sigLoadStarted(Movie*);
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
    QSet<MovieScraperInfo> m_infosToLoad;
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
