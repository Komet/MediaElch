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
    void addDownload(DownloadManagerElement elem);
    void setDownloads(QVector<DownloadManagerElement> elements);
    void abortDownloads();
    bool isDownloading();
    int downloadQueueSize();
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
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished();
    void startNextDownload();
    void downloadTimeout();

private:
    template<class T>
    int getNumberOfDownloadsLeft(T*& element);

    void checkAllMovieDownloadsFinished();
    void checkAllTvShowDownloadsFinished();
    void checkAllConcertDownloadsFinished();
    void checkAllArtistDownloadsFinished();
    void checkAllAlbumDownloadsFinished();

    mediaelch::network::NetworkManager* network();
    bool isLocalFile(const QUrl& url) const;

    QNetworkReply* m_currentReply = nullptr;
    DownloadManagerElement m_currentDownloadElement;
    QQueue<DownloadManagerElement> m_queue;
    // \todo Refactor into atomic
    bool m_downloading = false;
    QMutex m_mutex;
    QTimer m_timer;
    int m_retries = 0;
};
