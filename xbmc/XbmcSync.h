#ifndef XBMCSYNC_H
#define XBMCSYNC_H

#include <QDialog>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "data/Movie.h"

namespace Ui {
class XbmcSync;
}

class XbmcSync : public QDialog
{
    Q_OBJECT

public:
    explicit XbmcSync(QWidget *parent = 0);
    ~XbmcSync();
    enum Elements {
        ElementMovies, ElementConcerts, ElementTvShows, ElementEpisodes
    };

public slots:
    int exec();

private slots:
    void startSync();
    void onMovieListFinished();
    void onConcertListFinished();
    void onTvShowListFinished();
    void onEpisodeListFinished();
    void onRemoveFinished();
    void onScanFinished();
    void onTimeout();
    void onDownloadProgress();

private:
    Ui::XbmcSync *ui;
    QNetworkAccessManager *m_qnam;
    QNetworkReply *m_moviesReply;
    QNetworkReply *m_concertsReply;
    QNetworkReply *m_showReply;
    QNetworkReply *m_episodeReply;
    QNetworkReply *m_reply;
    QList<Movie*> m_moviesToSync;
    QList<Concert*> m_concertsToSync;
    QList<TvShow*> m_tvShowsToSync;
    QList<TvShowEpisode*> m_episodesToSync;
    QList<Elements> m_elements;
    QMap<int, QString> m_xbmcMovies;
    QMap<int, QString> m_xbmcConcerts;
    QMap<int, QString> m_xbmcShows;
    QMap<int, QString> m_xbmcEpisodes;
    QList<int> m_moviesToRemove;
    QList<int> m_concertsToRemove;
    QList<int> m_tvShowsToRemove;
    QList<int> m_episodesToRemove;
    QMutex m_mutex;
    bool m_allReady;
    QNetworkRequest m_request;
    QTimer m_timer;
    bool m_aborted;

    int findId(QStringList files, QMap<int, QString> items);
    bool compareFiles(QStringList files, QStringList xbmcFiles, int level);
    QStringList splitFile(QString file);
    void removeItems();
    void checkIfListsReady(Elements element);
    void triggerReload();
};

#endif // XBMCSYNC_H
