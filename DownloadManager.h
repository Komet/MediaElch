#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QQueue>
#include <QObject>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
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
    
signals:
    void downloadProgress(DownloadManagerElement);
    void downloadsLeft(int);
    void downloadFinished(DownloadManagerElement);
    void allDownloadsFinished();

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
};

#endif // DOWNLOADMANAGER_H
