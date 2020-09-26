#include "globals/DownloadManager.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include "globals/DownloadManagerElement.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "network/NetworkRequest.h"
#include "tv_shows/TvShow.h"

DownloadManager::DownloadManager(QObject* parent) : QObject(parent), m_downloading{false}
{
    connect(&m_timer, &QTimer::timeout, this, &DownloadManager::downloadTimeout);
}

/// \brief Returns the network access manager
/// \return Network access manager object
mediaelch::network::NetworkManager* DownloadManager::network()
{
    static auto* s_network = new mediaelch::network::NetworkManager();
    return s_network;
}

bool DownloadManager::isLocalFile(const QUrl& url) const
{
    return url.toString().startsWith("//");
}

/// \brief Add the given download element and start downloading it if the
///        download progress hasn't started, yet.
/// \param elem Element to download
/// \see   DownloadManagerElement
void DownloadManager::addDownload(DownloadManagerElement elem)
{
    qDebug() << "[DownloadManager] Enqueue download |" << elem.url;

    QMutexLocker locker(&m_mutex);
    const bool startDownloading = m_queue.isEmpty() && !m_downloading;
    m_queue.enqueue(elem);
    locker.unlock();

    if (startDownloading) {
        startNextDownload();
    }
}

/// \brief Aborts and clears all downloads and sets a list of new downloads
/// \param elements List of elements to download
/// \see   DownloadManagerElement
void DownloadManager::setDownloads(QVector<DownloadManagerElement> elements)
{
    QMutexLocker locker(&m_mutex);
    if (m_downloading) {
        locker.unlock(); // unlock for abort() call that may call downloadFinished()
        m_currentReply->abort();
    }

    m_timer.stop();
    m_queue.clear();

    locker.unlock();
    for (const DownloadManagerElement& elem : elements) {
        addDownload(elem);
    }

    locker.relock();
    const bool isEmpty = m_queue.isEmpty();
    locker.unlock();

    if (isEmpty) {
        QTimer::singleShot(0, this, &DownloadManager::allDownloadsFinished);
    }
}

/// Checks if all downloads of the current movie/tvshow/... have finished.
template<class T>
int DownloadManager::getNumberOfDownloadsLeft(T*& element)
{
    int numDownloadsLeft = 0;
    for (int i = 0, n = m_queue.size(); i < n; ++i) {
        T* currentDownload = m_currentDownloadElement.getElement<T>();
        if (currentDownload != nullptr && m_queue[i].getElement<T>() == currentDownload) {
            numDownloadsLeft++;
        }
    }
    if (numDownloadsLeft == 0) {
        element = m_currentDownloadElement.getElement<T>();
    }
    return numDownloadsLeft;
}

void DownloadManager::checkAllMovieDownloadsFinished()
{
    QMutexLocker locker(&m_mutex);
    Movie* movie = nullptr;
    int numDownloadsLeft = getNumberOfDownloadsLeft<Movie>(movie);
    if (numDownloadsLeft == 0) {
        locker.unlock();
        emit allMovieDownloadsFinished(movie);
    }
}

void DownloadManager::checkAllTvShowDownloadsFinished()
{
    QMutexLocker locker(&m_mutex);
    TvShow* tvShow = nullptr;
    int numDownloadsLeft = getNumberOfDownloadsLeft<TvShow>(tvShow);
    if (numDownloadsLeft == 0) {
        locker.unlock();
        emit allTvShowDownloadsFinished(tvShow);
    }
}

void DownloadManager::checkAllConcertDownloadsFinished()
{
    QMutexLocker locker(&m_mutex);
    Concert* concert = nullptr;
    int numDownloadsLeft = getNumberOfDownloadsLeft<Concert>(concert);
    if (numDownloadsLeft == 0) {
        locker.unlock();
        emit allConcertDownloadsFinished(concert);
    }
}

void DownloadManager::checkAllArtistDownloadsFinished()
{
    QMutexLocker locker(&m_mutex);
    Artist* artist = nullptr;
    int numDownloadsLeft = getNumberOfDownloadsLeft<Artist>(artist);
    if (numDownloadsLeft == 0) {
        locker.unlock();
        emit allArtistDownloadsFinished(artist);
    }
}

void DownloadManager::checkAllAlbumDownloadsFinished()
{
    QMutexLocker locker(&m_mutex);
    Album* album = nullptr;
    int numDownloadsLeft = getNumberOfDownloadsLeft<Album>(album);
    if (numDownloadsLeft == 0) {
        locker.unlock();
        emit allAlbumDownloadsFinished(album);
    }
}

void DownloadManager::startNextDownload()
{
    QMutexLocker locker(&m_mutex);
    if (m_downloading) {
        qWarning() << "[DownloadManager] Can't start another download in parallel; "
                      "this should not happen and may be a bug";
        return;
    }

    qDebug() << "[DownloadManager] Start next download";

    DownloadManagerElement oldDownload = m_currentDownloadElement;
    m_timer.stop();
    locker.unlock();

    if (oldDownload.movie != nullptr) {
        checkAllMovieDownloadsFinished();
    }
    if (oldDownload.show != nullptr) {
        checkAllTvShowDownloadsFinished();
    }
    if (oldDownload.concert != nullptr) {
        checkAllConcertDownloadsFinished();
    }
    if (oldDownload.artist != nullptr) {
        checkAllArtistDownloadsFinished();
    }
    if (oldDownload.album != nullptr) {
        checkAllAlbumDownloadsFinished();
    }

    locker.relock();
    if (m_queue.isEmpty()) {
        qDebug() << "[DownloadManager] All downloads finished";
        locker.unlock();
        emit allDownloadsFinished();
        return;
    }

    m_timer.start(8000);

    m_currentDownloadElement = m_queue.dequeue();
    DownloadManagerElement download = m_currentDownloadElement;

    if (download.imageType == ImageType::Actor || download.imageType == ImageType::TvShowEpisodeThumb) {
        if (download.movie != nullptr) {
            int numDownloadsLeft = 0;
            for (int i = 0, n = m_queue.size(); i < n; ++i) {
                if (m_queue[i].movie == download.movie) {
                    numDownloadsLeft++;
                }
            }
            locker.unlock();
            emit movieDownloadsLeft(numDownloadsLeft, download);

        } else if (download.show != nullptr) {
            int numDownloadsLeft = 0;
            for (int i = 0, n = m_queue.size(); i < n; ++i) {
                if (m_queue[i].show == download.show) {
                    numDownloadsLeft++;
                }
            }
            locker.unlock();
            emit showDownloadsLeft(numDownloadsLeft, download);

        } else {
            locker.unlock();
            emit downloadsLeft(m_queue.size());
        }
    }

    locker.unlock();

    if (isLocalFile(download.url)) {
        QFile file(download.url.toString());
        QByteArray data;
        if (file.open(QIODevice::ReadOnly)) {
            data = file.readAll();
            file.close();
        }

        download.data = data;

        if (download.actor != nullptr && download.imageType == ImageType::Actor && (download.movie == nullptr)) {
            download.actor->image = data;

        } else if (download.imageType == ImageType::TvShowEpisodeThumb && !download.directDownload) {
            download.episode->setThumbnailImage(data);

        } else {
            locker.unlock();
            emit sigDownloadFinished(download);
        }
        m_downloading = false;
        locker.unlock();
        startNextDownload();

    } else {
        locker.relock();
        m_downloading = true;
        QNetworkReply* reply = network()->get(mediaelch::network::requestWithDefaults(download.url));
        m_currentReply = reply;
        locker.unlock();

        connect(reply, &QNetworkReply::finished, this, &DownloadManager::downloadFinished);
        connect(reply, &QNetworkReply::downloadProgress, this, &DownloadManager::downloadProgress);
    }
}

/// \brief Called by the current network reply
/// \param received Received bytes
/// \param total Total bytes
void DownloadManager::downloadProgress(qint64 received, qint64 total)
{
    DownloadManagerElement element;
    {
        QMutexLocker locker(&m_mutex);
        m_currentDownloadElement.bytesReceived = received;
        m_currentDownloadElement.bytesTotal = total;
        element = m_currentDownloadElement;
        m_timer.start(5000);
    }
    emit sigDownloadProgress(element);
}

/// \brief Stops the current download and prepends it to the queue
void DownloadManager::downloadTimeout()
{
    QMutexLocker locker(&m_mutex);

    if (!m_downloading) {
        qCritical() << "[DownloadManager] Timeout on download even though nothing is downloading. Please report.";
        return;
    }

    QNetworkReply* reply = m_currentReply;
    auto download = m_currentDownloadElement;
    ++download.retries;

    qWarning() << "[DownloadManager] Download timed out:" << m_currentDownloadElement.url;

    // abort() calls downloadFinished() which would result in a deadlock if we still had the lock
    locker.unlock();
    reply->abort();
    reply->deleteLater();

    // It is possible that another download was started while processing this reply.
    // That is why we use the local copies of the download and "retries".
    locker.relock();

    if (download.retries < 3) {
        qDebug() << "[DownloadManager] Re-enqueuing the download, tries:" << download.retries << "/ 3";
        m_queue.prepend(download);

    } else {
        qDebug() << "[DownloadManager] Giving up on this file, tried 3 times";
    }

    if (!m_downloading) {
        locker.unlock();
        startNextDownload();
    }
}

/// \brief Called by the current network reply.
///        Starts the next download if there is one.
void DownloadManager::downloadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[DownloadManager] dynamic_cast<QNetworkReply*> failed!";
        return;
    }
    reply->deleteLater();

    QMutexLocker locker(&m_mutex);
    m_downloading = false;

    QByteArray data;
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[DownloadManager] Network Error:" << reply->errorString() << "|" << m_currentReply->url();
    } else {
        data = reply->readAll();
    }

    m_currentDownloadElement.data = data;

    if (m_currentDownloadElement.actor != nullptr && m_currentDownloadElement.imageType == ImageType::Actor
        && m_currentDownloadElement.movie == nullptr) {
        m_currentDownloadElement.actor->image = data;

    } else if (m_currentDownloadElement.imageType == ImageType::TvShowEpisodeThumb
               && !m_currentDownloadElement.directDownload) {
        m_currentDownloadElement.episode->setThumbnailImage(data);

    } else {
        DownloadManagerElement copy = m_currentDownloadElement;
        locker.unlock();
        emit sigDownloadFinished(copy);
    }

    DownloadManagerElement copy = m_currentDownloadElement;
    locker.unlock();
    emit sigElemDownloaded(copy);
    startNextDownload();
}

/// \brief Aborts the current download and clears the queue
void DownloadManager::abortDownloads()
{
    bool downloading = false;
    {
        qDebug() << "[DownloadsManager] Abort Downloads";
        QMutexLocker locker(&m_mutex);
        m_timer.stop();
        m_queue.clear();
        downloading = m_downloading;
    }
    if (downloading) {
        m_currentReply->abort();
    }
}

/**
 * \brief Check if a download is in progress
 * \return True if there is a download in progress
 */
bool DownloadManager::isDownloading()
{
    return m_downloading;
}

/**
 * \brief Returns the number of elements in queue
 * \return Number of elements in queue
 */
int DownloadManager::downloadQueueSize()
{
    return m_queue.size();
}

/**
 * \brief Returns the number of left downloads for a TV show
 * \param show Tv show to get number of downloads for
 * \return Number of downloads left
 */
int DownloadManager::downloadsLeftForShow(TvShow* show)
{
    int left = 0;
    m_mutex.lock();
    for (int i = 0, n = m_queue.count(); i < n; ++i) {
        if (m_queue[i].show == show) {
            left++;
        }
    }
    m_mutex.unlock();
    qDebug() << "[DownloadManager] Downloads left for show " << show->title() << ":" << left;
    return left;
}
