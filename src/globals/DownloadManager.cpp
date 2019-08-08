#include "globals/DownloadManager.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include "globals/DownloadManagerElement.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "tv_shows/TvShow.h"

DownloadManager::DownloadManager(QObject* parent) : QObject(parent), m_downloading{false}
{
    connect(&m_timer, &QTimer::timeout, this, &DownloadManager::downloadTimeout);
}

/**
 * @brief Returns the network access manager
 * @return Network access manager object
 */
QNetworkAccessManager* DownloadManager::qnam()
{
    static auto s_qnam = new QNetworkAccessManager();
    return s_qnam;
}

/// @brief Add the given download element and start downloading it if the
///        download progress hasn't started, yet.
/// @param elem Element to download
/// @see   DownloadManagerElement
void DownloadManager::addDownload(DownloadManagerElement elem)
{
    qDebug() << "Enqueue download | " << elem.url;

    bool startDownloading = false;
    {
        QMutexLocker locker(&m_mutex);
        startDownloading = m_queue.isEmpty() && !m_downloading;
        m_queue.enqueue(elem);
    }

    if (startDownloading) {
        startNextDownload();
    }
}

/// @brief Aborts and clears all downloads and sets a list of new downloads
/// @param elements List of elements to download
/// @see   DownloadManagerElement
void DownloadManager::setDownloads(QVector<DownloadManagerElement> elements)
{
    if (m_downloading) {
        m_currentReply->abort();
    }

    m_timer.stop();

    {
        QMutexLocker locker(&m_mutex);
        m_queue.clear();
    }

    for (const DownloadManagerElement& elem : elements) {
        addDownload(elem);
    }

    QMutexLocker locker(&m_mutex);
    if (m_queue.isEmpty()) {
        QTimer::singleShot(0, this, SIGNAL(allDownloadsFinished()));
    }
}

/// Checks if all downloads of the current movie/tvshow/... have finished.
template<class T>
void DownloadManager::checkAllDownloadsFinished()
{
    int numDownloadsLeft = 0;
    for (int i = 0, n = m_queue.size(); i < n; ++i) {
        if (m_queue[i].getElement<T>() == m_currentDownloadElement.getElement<T>()) {
            numDownloadsLeft++;
        }
    }
    if (numDownloadsLeft == 0) {
        emit allDownloadsFinished(m_currentDownloadElement.getElement<T>());
    }
}

/**
 * @brief Starts the next download
 */
void DownloadManager::startNextDownload()
{
    m_timer.stop();
    if (m_currentDownloadElement.movie != nullptr) {
        checkAllDownloadsFinished<Movie>();
    }
    if (m_currentDownloadElement.show != nullptr) {
        checkAllDownloadsFinished<TvShow>();
    }
    if (m_currentDownloadElement.concert != nullptr) {
        checkAllDownloadsFinished<Concert>();
    }
    if (m_currentDownloadElement.artist != nullptr) {
        checkAllDownloadsFinished<Artist>();
    }
    if (m_currentDownloadElement.album != nullptr) {
        checkAllDownloadsFinished<Album>();
    }

    if (m_queue.isEmpty()) {
        qDebug() << "All downloads finished";
        emit allDownloadsFinished();
        return;
    }

    m_timer.start(8000);
    m_downloading = true;
    {
        QMutexLocker locker(&m_mutex);
        m_currentDownloadElement = m_queue.dequeue();
    }

    if (!m_currentDownloadElement.url.toString().startsWith("//")) {
        m_currentReply = qnam()->get(QNetworkRequest(m_currentDownloadElement.url));
        connect(m_currentReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        connect(m_currentReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
    }

    QMutexLocker locker(&m_mutex);
    if (m_currentDownloadElement.imageType == ImageType::Actor
        || m_currentDownloadElement.imageType == ImageType::TvShowEpisodeThumb) {
        if (m_currentDownloadElement.movie != nullptr) {
            int numDownloadsLeft = 0;
            for (int i = 0, n = m_queue.size(); i < n; ++i) {
                if (m_queue[i].movie == m_currentDownloadElement.movie) {
                    numDownloadsLeft++;
                }
            }
            locker.unlock();
            emit downloadsLeft(numDownloadsLeft, m_currentDownloadElement);
        } else if (m_currentDownloadElement.show != nullptr) {
            int numDownloadsLeft = 0;
            for (int i = 0, n = m_queue.size(); i < n; ++i) {
                if (m_queue[i].show == m_currentDownloadElement.show) {
                    numDownloadsLeft++;
                }
            }
            locker.unlock();
            emit downloadsLeft(numDownloadsLeft, m_currentDownloadElement);
        } else {
            locker.unlock();
            emit downloadsLeft(m_queue.size());
        }
    }

    if (m_currentDownloadElement.url.toString().startsWith("//")) {
        QFile file(m_currentDownloadElement.url.toString());
        QByteArray data;
        if (file.open(QIODevice::ReadOnly)) {
            data = file.readAll();
            file.close();
        }

        locker.relock();
        m_currentDownloadElement.data = data;

        if (m_currentDownloadElement.actor != nullptr && m_currentDownloadElement.imageType == ImageType::Actor
            && (m_currentDownloadElement.movie == nullptr)) {
            m_currentDownloadElement.actor->image = data;
            locker.unlock();

        } else if (m_currentDownloadElement.imageType == ImageType::TvShowEpisodeThumb
                   && !m_currentDownloadElement.directDownload) {
            m_currentDownloadElement.episode->setThumbnailImage(data);
            locker.unlock();

        } else {
            locker.unlock();

            emit sigDownloadFinished(m_currentDownloadElement);
        }
        startNextDownload();
    }
}

/**
 * @brief Called by the current network reply
 * @param received Received bytes
 * @param total Total bytes
 */
void DownloadManager::downloadProgress(qint64 received, qint64 total)
{
    m_timer.start(5000);
    m_currentDownloadElement.bytesReceived = received;
    m_currentDownloadElement.bytesTotal = total;
    emit downloadProgress(m_currentDownloadElement);
}

/**
 * @brief Stops the current download and prepends it to the queue
 */
void DownloadManager::downloadTimeout()
{
    if (!m_downloading) {
        return;
    }
    qWarning() << "Download timed out" << m_currentDownloadElement.url;
    m_retries++;
    m_currentReply->abort();
    m_currentReply->deleteLater();
    if (m_retries <= 2) {
        qDebug() << "Restarting the download";
        m_queue.prepend(m_currentDownloadElement);

    } else {
        qDebug() << "Giving up on this file, tried 3 times";
        m_retries = 0;
    }
    startNextDownload();
}

/**
 * @brief Called by the current network reply
 * Starts the next download if there is one
 */
void DownloadManager::downloadFinished()
{
    qDebug() << "Entered";

    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302
        || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        reply->deleteLater();
        m_currentReply =
            qnam()->get(QNetworkRequest(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
        connect(m_currentReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        connect(m_currentReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
        return;
    }

    m_downloading = false;
    m_retries = 0;
    QByteArray data;
    if (m_currentReply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error:" << m_currentReply->errorString() << "|" << m_currentReply->url();
    } else {
        data = m_currentReply->readAll();
    }

    {
        QMutexLocker locker(&m_mutex);
        m_currentDownloadElement.data = data;
        reply->deleteLater();
    }

    QMutexLocker locker(&m_mutex);
    if (m_currentDownloadElement.actor != nullptr                 //
        && m_currentDownloadElement.imageType == ImageType::Actor //
        && m_currentDownloadElement.movie == nullptr) {
        m_currentDownloadElement.actor->image = data;
        locker.unlock();

    } else if (m_currentDownloadElement.imageType == ImageType::TvShowEpisodeThumb
               && !m_currentDownloadElement.directDownload) {
        m_currentDownloadElement.episode->setThumbnailImage(data);
        locker.unlock();

    } else {
        locker.unlock();
        emit sigDownloadFinished(m_currentDownloadElement);
    }

    emit sigElemDownloaded(m_currentDownloadElement);
    startNextDownload();
}

/**
 * @brief Aborts the current download and clears the queue
 */
void DownloadManager::abortDownloads()
{
    qDebug() << "Entered";
    m_timer.stop();
    m_mutex.lock();
    m_queue.clear();
    m_mutex.unlock();
    if (m_downloading) {
        m_currentReply->abort();
    }
}

/**
 * @brief Check if a download is in progress
 * @return True if there is a download in progress
 */
bool DownloadManager::isDownloading()
{
    return m_downloading;
}

/**
 * @brief Returns the number of elements in queue
 * @return Number of elements in queue
 */
int DownloadManager::downloadQueueSize()
{
    return m_queue.size();
}

/**
 * @brief Returns the number of left downloads for a tv show
 * @param show Tv show to get number of downloads for
 * @return Number of downloads left
 */
int DownloadManager::downloadsLeftForShow(TvShow* show)
{
    qDebug() << "Entered, show=" << show->name();
    int left = 0;
    m_mutex.lock();
    for (int i = 0, n = m_queue.count(); i < n; ++i) {
        if (m_queue[i].show == show) {
            left++;
        }
    }
    m_mutex.unlock();
    qDebug() << "Downloads left" << left;
    return left;
}
