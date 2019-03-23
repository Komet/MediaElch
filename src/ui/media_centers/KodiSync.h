#pragma once

#include "movies/Movie.h"
#include "settings/KodiSettings.h"

#include <QAuthenticator>
#include <QDialog>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpSocket>
#include <QTimer>
#include <QUrl>

namespace Ui {
class KodiSync;
}

class KodiSync : public QDialog
{
    Q_OBJECT

public:
    explicit KodiSync(KodiSettings& settings, QWidget* parent = nullptr);
    ~KodiSync() override;
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
    void onAuthRequired(QNetworkReply* reply, QAuthenticator* authenticator);

private:
    Ui::KodiSync* ui;
    KodiSettings& m_settings;

    QNetworkAccessManager m_qnam;
    QVector<Movie*> m_moviesToSync;
    QVector<Concert*> m_concertsToSync;
    QVector<TvShow*> m_tvShowsToSync;
    QVector<TvShowEpisode*> m_episodesToSync;
    QVector<Element> m_elements;
    QMap<int, XbmcData> m_xbmcMovies;
    QMap<int, XbmcData> m_xbmcConcerts;
    QMap<int, XbmcData> m_xbmcShows;
    QMap<int, XbmcData> m_xbmcEpisodes;
    QVector<int> m_moviesToRemove;
    QVector<int> m_concertsToRemove;
    QVector<int> m_tvShowsToRemove;
    QVector<int> m_episodesToRemove;
    QMutex m_mutex;
    bool m_allReady;
    bool m_aborted;
    SyncType m_syncType;
    bool m_cancelRenameArtwork;
    bool m_renameArtworkInProgress;
    bool m_artworkWasRenamed;
    int m_reloadTimeOut;
    int m_requestId;

    int findId(const QStringList& files, const QMap<int, XbmcData>& items);
    bool compareFiles(const QStringList& files, const QStringList& xbmcFiles, const int& level);
    QStringList splitFile(const QString& file);
    void setupItemsToRemove();
    void removeItems();
    void updateWatched();
    void checkIfListsReady(Element element);
    KodiSync::XbmcData parseXbmcDataFromMap(QMap<QString, QVariant> map);
    void updateFolderLastModified(Movie* movie);
    void updateFolderLastModified(Concert* concert);
    void updateFolderLastModified(TvShow* show);
    void updateFolderLastModified(TvShowEpisode* episode);
    QUrl xbmcUrl();
};
