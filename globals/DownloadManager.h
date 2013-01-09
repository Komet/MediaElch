#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QMutex>
#include <QQueue>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include "globals/DownloadManagerElement.h"
#include "globals/Globals.h"
#include "data/TvShowEpisode.h"

/**
 * @brief The DownloadManager class
 */
class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = 0);
    void addDownload(DownloadManagerElement elem);
    void setDownloads(QList<DownloadManagerElement> elements);
    void abortDownloads();
    bool isDownloading();
    int downloadQueueSize();
    int downloadsLeftForShow(TvShow *show);

signals:
    void downloadProgress(DownloadManagerElement);
    void downloadsLeft(int);
    void downloadsLeft(int, DownloadManagerElement);
    void downloadFinished(DownloadManagerElement);
    void allDownloadsFinished();
    void allDownloadsFinished(Movie*);
    void allDownloadsFinished(TvShow*);
    void allDownloadsFinished(Concert*);

private slots:
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished();
    void startNextDownload();
    void downloadTimeout();

private:
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_currentReply;
    DownloadManagerElement m_currentDownloadElement;
    QQueue<DownloadManagerElement> m_queue;
    QNetworkAccessManager *qnam();
    bool m_downloading;
    QMutex m_mutex;
    QTimer m_timer;
    int m_retries;
};

#endif // DOWNLOADMANAGER_H
