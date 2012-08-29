#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QMutex>
#include <QQueue>
#include <QObject>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include "DownloadManagerElement.h"
#include "Globals.h"
#include "data/TvShowEpisode.h"

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

private slots:
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished();
    void startNextDownload();

private:
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_currentReply;
    DownloadManagerElement m_currentDownloadElement;
    QQueue<DownloadManagerElement> m_queue;
    QNetworkAccessManager *qnam();
    bool m_downloading;
    QMutex m_mutex;
};

#endif // DOWNLOADMANAGER_H
