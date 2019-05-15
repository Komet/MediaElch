#pragma once

#include "globals/DownloadManagerElement.h"
#include "globals/Globals.h"

#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QUrl>
#include <QVector>

class Artist;
class Album;

/**
 * @brief The DownloadManager class
 */
class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject* parent = nullptr);
    void addDownload(DownloadManagerElement elem);
    void setDownloads(QVector<DownloadManagerElement> elements);
    void abortDownloads();
    bool isDownloading();
    int downloadQueueSize();
    int downloadsLeftForShow(TvShow* show);

signals:
    void downloadProgress(DownloadManagerElement);
    void downloadsLeft(int);
    void downloadsLeft(int, DownloadManagerElement);
    void sigDownloadFinished(DownloadManagerElement);
    void sigElemDownloaded(DownloadManagerElement);
    void allDownloadsFinished();
    void allDownloadsFinished(Movie*);
    void allDownloadsFinished(TvShow*);
    void allDownloadsFinished(Concert*);
    void allDownloadsFinished(Artist*);
    void allDownloadsFinished(Album*);

private slots:
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished();
    void startNextDownload();
    void downloadTimeout();

private:
    template<class T>
    void startNextDownloadType();

    QNetworkReply* m_currentReply;
    DownloadManagerElement m_currentDownloadElement;
    QQueue<DownloadManagerElement> m_queue;
    QNetworkAccessManager* qnam();
    bool m_downloading;
    QMutex m_mutex;
    QTimer m_timer;
    int m_retries;
};
