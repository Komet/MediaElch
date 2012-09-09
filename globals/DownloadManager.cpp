#include "globals/DownloadManager.h"

#include <QDebug>
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
}

/**
 * @brief Returns the network access manager
 * @return Network access manager object
 */
QNetworkAccessManager *DownloadManager::qnam()
{
    return &m_qnam;
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
    qDebug() << "Entered";
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

    if (m_queue.isEmpty()) {
        qDebug() << "All downloads finished";
        emit allDownloadsFinished();
        return;
    }

    m_downloading = true;
    m_mutex.lock();
    m_currentDownloadElement = m_queue.dequeue();
    m_mutex.unlock();
    m_currentReply = this->qnam()->get(QNetworkRequest(m_currentDownloadElement.url));
    connect(m_currentReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(m_currentReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));


    if (m_currentDownloadElement.imageType == TypeActor || m_currentDownloadElement.imageType == TypeShowThumbnail) {
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
}

/**
 * @brief Called by the current network reply
 * @param received Received bytes
 * @param total Total bytes
 */
void DownloadManager::downloadProgress(qint64 received, qint64 total)
{
    m_currentDownloadElement.bytesReceived = received;
    m_currentDownloadElement.bytesTotal = total;
    emit downloadProgress(m_currentDownloadElement);
}

/**
 * @brief Called by the current network reply
 * Starts the next download if there is one
 */
void DownloadManager::downloadFinished()
{
    qDebug() << "Entered";
    m_downloading = false;
    if (this->m_currentReply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << m_currentReply->errorString();
        return;
    }
    QImage img;
    img.loadFromData(m_currentReply->readAll());
    m_currentDownloadElement.image = img;
    m_currentReply->deleteLater();
    if (m_currentDownloadElement.imageType == TypeActor)
        m_currentDownloadElement.actor->image = img;
    else if (m_currentDownloadElement.imageType == TypeShowThumbnail)
        m_currentDownloadElement.episode->setThumbnailImage(img);
    else
        emit downloadFinished(m_currentDownloadElement);
    this->startNextDownload();
}

/**
 * @brief Aborts the current download and clears the queue
 */
void DownloadManager::abortDownloads()
{
    qDebug() << "Entered";
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
