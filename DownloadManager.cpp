#include "DownloadManager.h"

#include <QDebug>
#include <QTimer>

#include "DownloadManagerElement.h"

DownloadManager::DownloadManager(QObject *parent) :
    QObject(parent)
{
    m_downloading = false;
}

QNetworkAccessManager *DownloadManager::qnam()
{
    return &m_qnam;
}

void DownloadManager::addDownload(DownloadManagerElement elem)
{
    if (m_queue.isEmpty())
        QTimer::singleShot(0, this, SLOT(startNextDownload()));
    m_mutex.lock();
    m_queue.enqueue(elem);
    m_mutex.unlock();
}

void DownloadManager::setDownloads(QList<DownloadManagerElement> elements)
{
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

void DownloadManager::startNextDownload()
{
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

void DownloadManager::downloadProgress(qint64 received, qint64 total)
{
    m_currentDownloadElement.bytesReceived = received;
    m_currentDownloadElement.bytesTotal = total;
    emit downloadProgress(m_currentDownloadElement);
}

void DownloadManager::downloadFinished()
{
    m_downloading = false;
    if (this->m_currentReply->error() != QNetworkReply::NoError)
        return;

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

void DownloadManager::abortDownloads()
{
    m_mutex.lock();
    m_queue.clear();
    m_mutex.unlock();
    if (m_downloading) {
        m_currentReply->abort();
    }
}

bool DownloadManager::isDownloading()
{
    return m_downloading;
}

int DownloadManager::downloadQueueSize()
{
    return m_queue.size();
}

int DownloadManager::downloadsLeftForShow(TvShow *show)
{
    int left = 0;
    m_mutex.lock();
    for (int i=0, n=m_queue.count() ; i<n ; ++i) {
        if (m_queue[i].show == show)
            left++;
    }
    m_mutex.unlock();
    return left;
}
