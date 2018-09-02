#ifndef XBMCSYNC_H
#define XBMCSYNC_H

#include <QAuthenticator>
#include <QDialog>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>

#include "data/Movie.h"

namespace Ui {
class XbmcSync;
}

class XbmcSync : public QDialog
{
    Q_OBJECT

public:
    explicit XbmcSync(QWidget *parent = nullptr);
    ~XbmcSync() override;
    enum class Element
    {
        Movies,
        Concerts,
        TvShows,
        Episodes
    };

    enum class SyncType
    {
        Contents,
        Watched,
        Clean
    };

    struct XbmcData
    {
        QString file;
        QDateTime lastPlayed;
        int playCount;
    };

public slots:
    int exec() override;
    void reject() override;

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
    void onCleanFinished();
    void onRadioContents();
    void onRadioClean();
    void onRadioWatched();
    void onButtonClose();
    void triggerReload();
    void triggerClean();
    void onAuthRequired(QNetworkReply *reply, QAuthenticator *authenticator);

private:
    Ui::XbmcSync *ui;

    QNetworkAccessManager m_qnam;
    QList<Movie *> m_moviesToSync;
    QList<Concert *> m_concertsToSync;
    QList<TvShow *> m_tvShowsToSync;
    QList<TvShowEpisode *> m_episodesToSync;
    QList<Element> m_elements;
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
    int m_reloadTimeOut;
    int m_requestId;

    int findId(const QStringList &files, const QMap<int, XbmcData> &items);
    bool compareFiles(const QStringList &files, const QStringList &xbmcFiles, const int &level);
    QStringList splitFile(const QString &file);
    void setupItemsToRemove();
    void removeItems();
    void updateWatched();
    void checkIfListsReady(Element element);
    XbmcSync::XbmcData parseXbmcDataFromMap(QMap<QString, QVariant> map);
    void updateFolderLastModified(Movie *movie);
    void updateFolderLastModified(Concert *concert);
    void updateFolderLastModified(TvShow *show);
    void updateFolderLastModified(TvShowEpisode *episode);
    QUrl xbmcUrl();
};

#endif // XBMCSYNC_H
