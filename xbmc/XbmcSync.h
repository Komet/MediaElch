#ifndef XBMCSYNC_H
#define XBMCSYNC_H

#include <QDialog>
#include <QMutex>
#include <QScriptValue>
#include <QTcpSocket>
#include <QTimer>
#include "movies/Movie.h"
#include "qjsonrpcservice.h"

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

    enum SyncType {
        SyncContents, SyncWatched, RenameArtwork
    };

    struct XbmcData {
        QString file;
        QDateTime lastPlayed;
        int playCount;
    };

public slots:
    int exec();
    void reject();

signals:
    void sigTriggerReload();
    void sigFinished();

private slots:
    void startSync();
    void onMovieListFinished();
    void onConcertListFinished();
    void onTvShowListFinished();
    void onEpisodeListFinished();
    void onRemoveFinished();
    void onScanFinished();
    void onRadioContents();
    void onRadioWatched();
    void onRadioRenameArtwork();
    void onButtonClose();
    void processMessage(QJsonRpcMessage msg);
    void triggerReload();

private:
    Ui::XbmcSync *ui;

    QTcpSocket *m_socket;
    QJsonRpcSocket *m_client;
    QList<Movie*> m_moviesToSync;
    QList<Concert*> m_concertsToSync;
    QList<TvShow*> m_tvShowsToSync;
    QList<TvShowEpisode*> m_episodesToSync;
    QList<Elements> m_elements;
    QMap<int, XbmcData> m_xbmcMovies;
    QMap<int, XbmcData> m_xbmcConcerts;
    QMap<int, XbmcData> m_xbmcShows;
    QMap<int, XbmcData> m_xbmcEpisodes;
    QList<int> m_moviesToRemove;
    QList<int> m_concertsToRemove;
    QList<int> m_tvShowsToRemove;
    QList<int> m_episodesToRemove;
    QMutex m_mutex;
    bool m_allReady;
    bool m_aborted;
    SyncType m_syncType;
    bool m_cancelRenameArtwork;
    bool m_renameArtworkInProgress;
    bool m_artworkWasRenamed;
    QStringList m_directoriesToSync;
    int m_reloadTimeOut;

    int findId(QStringList files, QMap<int, XbmcData> items);
    bool compareFiles(QStringList files, QStringList xbmcFiles, int level);
    QStringList splitFile(QString file);
    void setupItemsToRemove();
    void removeItems();
    void updateWatched();
    void checkIfListsReady(Elements element);
    XbmcSync::XbmcData parseXbmcDataFromMap(QMap<QString, QVariant> map);
    void renameArtwork();
};

#endif // XBMCSYNC_H
