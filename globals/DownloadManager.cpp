#include "globals/DownloadManager.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include "globals/DownloadManagerElement.h"

/**
 * @brief DownloadManager::DownloadManager
 * @param parent
 */
DownloadManager::DownloadManager(QObject *parent) :
    QObject(parent)
{
    m_downloading = false;
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(downloadTimeout()));
}

/**
 * @brief Returns the network access manager
 * @return Network access manager object
 */
QNetworkAccessManager *DownloadManager::qnam()
{
    static QNetworkAccessManager *qnam = new QNetworkAccessManager();
    return qnam;
}

/**
 * @brief Adds a download and starts downloading
 * @param elem Element to download
 * @see DownloadManagerElement
 */
void DownloadManager::addDownload(DownloadManagerElement elem)
{
    qDebug() << "Entered, url=" << elem.url;
    if (m_queue.isEmpty())
        QTimer::singleShot(0, this, SLOT(startNextDownload()));
    m_mutex.lock();
    m_queue.enqueue(elem);
    m_mutex.unlock();
}

/**
 * @brief Aborts and clears all downloads and sets a list of new downloads
 * @param elements List of elements to download
 * @see DownloadManagerElement
 */
void DownloadManager::setDownloads(QList<DownloadManagerElement> elements)
{
    qDebug() << "Entered";
    if (m_downloading)
        m_currentReply->abort();

    m_timer.stop();
    m_mutex.lock();
    m_queue.clear();
    m_mutex.unlock();

    foreach (const DownloadManagerElement &elem, elements)
        addDownload(elem);

    if (m_queue.isEmpty())
        QTimer::singleShot(0, this, SIGNAL(allDownloadsFinished()));
}

/**
 * @brief Starts the next download
 */
void DownloadManager::startNextDownload()
{
    m_timer.stop();
    if (m_currentDownloadElement.movie) {
        int numDownloadsLeft = 0;
        for (int i=0, n=m_queue.size() ; i<n ; ++i) {
            if (m_queue[i].movie == m_currentDownloadElement.movie)
                numDownloadsLeft++;
        }
        if (numDownloadsLeft == 0)
            emit allDownloadsFinished(m_currentDownloadElement.movie);
    }

    if (m_currentDownloadElement.show) {
        int numDownloadsLeft = 0;
        for (int i=0, n=m_queue.size() ; i<n ; ++i) {
            if (m_queue[i].show == m_currentDownloadElement.show)
                numDownloadsLeft++;
        }
        if (numDownloadsLeft == 0)
            emit allDownloadsFinished(m_currentDownloadElement.show);
    }

    if (m_currentDownloadElement.concert) {
        int numDownloadsLeft = 0;
        for (int i=0, n=m_queue.size() ; i<n ; ++i) {
            if (m_queue[i].concert == m_currentDownloadElement.concert)
                numDownloadsLeft++;
        }
        if (numDownloadsLeft == 0)
            emit allDownloadsFinished(m_currentDownloadElement.concert);
    }

    if (m_currentDownloadElement.artist) {
        int numDownloadsLeft = 0;
        for (int i=0, n=m_queue.size() ; i<n ; ++i) {
            if (m_queue[i].artist == m_currentDownloadElement.artist)
                numDownloadsLeft++;
        }
        if (numDownloadsLeft == 0)
            emit allDownloadsFinished(m_currentDownloadElement.artist);
    }

    if (m_currentDownloadElement.album) {
        int numDownloadsLeft = 0;
        for (int i=0, n=m_queue.size() ; i<n ; ++i) {
            if (m_queue[i].album == m_currentDownloadElement.album)
                numDownloadsLeft++;
        }
        if (numDownloadsLeft == 0)
            emit allDownloadsFinished(m_currentDownloadElement.album);
    }

    if (m_queue.isEmpty()) {
        qDebug() << "All downloads finished";
        emit allDownloadsFinished();
        return;
    }

    m_timer.start(8000);
    m_downloading = true;
    m_mutex.lock();
    m_currentDownloadElement = m_queue.dequeue();
    m_mutex.unlock();
    if (!m_currentDownloadElement.url.toString().startsWith("//")) {
        m_currentReply = qnam()->get(QNetworkRequest(m_currentDownloadElement.url));
        connect(m_currentReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        connect(m_currentReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    }

    if (m_currentDownloadElement.imageType == ImageType::Actor || m_currentDownloadElement.imageType == ImageType::TvShowEpisodeThumb) {
        if (m_currentDownloadElement.movie) {
            int numDownloadsLeft = 0;
            m_mutex.lock();
            for (int i=0, n=m_queue.size() ; i<n ; ++i) {
                if (m_queue[i].movie == m_currentDownloadElement.movie)
                    numDownloadsLeft++;
            }
            m_mutex.unlock();
            emit downloadsLeft(numDownloadsLeft, m_currentDownloadElement);
        } else if (m_currentDownloadElement.show) {
            int numDownloadsLeft = 0;
            m_mutex.lock();
            for (int i=0, n=m_queue.size() ; i<n ; ++i) {
                if (m_queue[i].show == m_currentDownloadElement.show)
                    numDownloadsLeft++;
            }
            m_mutex.unlock();
            emit downloadsLeft(numDownloadsLeft, m_currentDownloadElement);
        } else {
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
        m_currentDownloadElement.data = data;
        if (m_currentDownloadElement.imageType == ImageType::Actor && !m_currentDownloadElement.movie)
            m_currentDownloadElement.actor->image = data;
        else if (m_currentDownloadElement.imageType == ImageType::TvShowEpisodeThumb && !m_currentDownloadElement.directDownload)
            m_currentDownloadElement.episode->setThumbnailImage(data);
        else
            emit downloadFinished(m_currentDownloadElement);
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
    if (!m_downloading)
        return;
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

    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 302 ||
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301) {
        reply->deleteLater();
        m_currentReply = qnam()->get(QNetworkRequest(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl()));
        connect(m_currentReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        connect(m_currentReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
        return;
    }

    m_downloading = false;
    m_retries = 0;
    QByteArray data;
    if (m_currentReply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << m_currentReply->errorString();
    } else {
        data = m_currentReply->readAll();
    }
    m_currentDownloadElement.data = data;
    reply->deleteLater();
    if (m_currentDownloadElement.imageType == ImageType::Actor && !m_currentDownloadElement.movie)
        m_currentDownloadElement.actor->image = data;
    else if (m_currentDownloadElement.imageType == ImageType::TvShowEpisodeThumb && !m_currentDownloadElement.directDownload)
        m_currentDownloadElement.episode->setThumbnailImage(data);
    else
        emit downloadFinished(m_currentDownloadElement);
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
int DownloadManager::downloadsLeftForShow(TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    int left = 0;
    m_mutex.lock();
    for (int i=0, n=m_queue.count() ; i<n ; ++i) {
        if (m_queue[i].show == show)
            left++;
    }
    m_mutex.unlock();
    qDebug() << "Downloads left" << left;
    return left;
}
