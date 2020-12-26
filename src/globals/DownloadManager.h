#pragma once

#include "globals/DownloadManagerElement.h"
#include "globals/Globals.h"
#include "network/NetworkManager.h"

#include <QMutex>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QUrl>
#include <QVector>

class Artist;
class Album;

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject* parent = nullptr);
    /// \brief Add the given download element and start downloading it if the
    ///        download progress hasn't started, yet.
    /// \param elem Element to download
    /// \see   DownloadManagerElement
    void addDownload(DownloadManagerElement elem);
    /// \brief Aborts and clears all downloads and sets a list of new downloads
    /// \param elements List of elements to download
    /// \see   DownloadManagerElement
    void setDownloads(QVector<DownloadManagerElement> elements);
    /// \brief Aborts the current download and clears the queue
    void abortDownloads();
    /// \brief Check if a download is in progress
    /// \return True if there is a download in progress
    bool isDownloading() const;
    /// \brief How many elements are either in the queue or are currently being downloaded.
    int downloadQueueSize();
    /// \brief Returns the number of left downloads for a TV show
    /// \param show Tv show to get number of downloads for
    /// \return Number of downloads left
    int downloadsLeftForShow(TvShow* show);

signals:
    void sigDownloadProgress(DownloadManagerElement);
    void downloadsLeft(int);
    void movieDownloadsLeft(int, DownloadManagerElement);
    void showDownloadsLeft(int, DownloadManagerElement);

    void sigDownloadFinished(DownloadManagerElement);
    void sigElemDownloaded(DownloadManagerElement);
    void allDownloadsFinished();
    void allMovieDownloadsFinished(Movie*);
    void allTvShowDownloadsFinished(TvShow*);
    void allConcertDownloadsFinished(Concert*);
    void allArtistDownloadsFinished(Artist*);
    void allAlbumDownloadsFinished(Album*);

private slots:
    /// \param received Received bytes
    /// \param total Total bytes
    void downloadProgress(qint64 received, qint64 total);
    /// \brief Starts the next download if there is one.
    void downloadFinished();
    void startNextDownload();
    /// \brief Stops the current download and prepends it to the queue
    void restartDownloadAfterTimeout(QNetworkReply* reply);

private:
    /// \brief Checks if all downloads of the given movie/tvshow/... have finished.
    template<class T>
    bool hasDownloadsLeft(T*& elementToCheck);
    /// \brief Count all downloads of the given movie/tvshow/... have finished.
    template<class T>
    int numberOfDownloadsLeft(T*& elementToCheck);

    /// \brief Returns the network access manager
    /// \return Network access manager object
    mediaelch::network::NetworkManager* network();
    static bool isLocalFile(const QUrl& url);

    QVector<QNetworkReply*> m_currentReplies;
    QQueue<DownloadManagerElement> m_queue;

    int numberOfParellelDownloads = 5;
};
