#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QPoint>
#include <QSettings>
#include <QSize>

#include "globals/Globals.h"
#include "settings/DataFile.h"

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);

    static Settings *instance(QObject *parent = 0);
    void loadSettings();

    QSize mainWindowSize();
    QPoint mainWindowPosition();
    QByteArray mainSplitterState();
    QList<SettingsDir> movieDirectories();
    QList<SettingsDir> tvShowDirectories();
    QList<SettingsDir> concertDirectories();
    int mediaCenterInterface();
    QString xbmcMysqlHost();
    QString xbmcMysqlDatabase();
    QString xbmcMysqlUser();
    QString xbmcMysqlPassword();
    QString xbmcSqliteDatabase();
    QString xbmcThumbnailPath();
    bool debugModeActivated();
    QString debugLogPath();
    bool useYoutubePluginUrls();
    QList<DataFile*> movieNfoFiles();
    QList<DataFile*> moviePosterFiles();
    QList<DataFile*> movieFanartFiles();
    QList<DataFile*> tvShowPosterFiles();
    QList<DataFile*> tvShowBannerFiles();
    QList<DataFile*> concertNfoFiles();
    QList<DataFile*> concertPosterFiles();
    QList<DataFile*> concertFanartFiles();
    QList<DataFile*> enabledMovieNfoFiles();
    QList<DataFile*> enabledMoviePosterFiles();
    QList<DataFile*> enabledMovieFanartFiles();
    QList<DataFile*> enabledTvShowPosterFiles();
    QList<DataFile*> enabledTvShowBannerFiles();
    QList<DataFile*> enabledConcertNfoFiles();
    QList<DataFile*> enabledConcertPosterFiles();
    QList<DataFile*> enabledConcertFanartFiles();
    bool useCache();

    void setMainWindowSize(QSize mainWindowSize);
    void setMainWindowPosition(QPoint mainWindowPosition);
    void setMainSplitterState(QByteArray state);
    void setMovieDirectories(QList<SettingsDir> dirs);
    void setTvShowDirectories(QList<SettingsDir> dirs);
    void setConcertDirectories(QList<SettingsDir> dirs);
    void setXbmcMysqlHost(QString host);
    void setXbmcMysqlDatabase(QString db);
    void setXbmcMysqlUser(QString user);
    void setXbmcMysqlPassword(QString password);
    void setUseYoutubePluginUrls(bool use);
    void setXbmcSqliteDatabase(QString file);
    void setXbmcThumbnailPath(QString path);
    void setDebugModeActivated(bool enabled);
    void setDebugLogPath(QString path);
    void setMovieNfoFiles(QList<DataFile*> files);
    void setMoviePosterFiles(QList<DataFile*> files);
    void setMovieFanartFiles(QList<DataFile*> files);
    void setTvShowPosterFiles(QList<DataFile*> files);
    void setTvShowBannerFiles(QList<DataFile*> files);
    void setConcertNfoFiles(QList<DataFile*> files);
    void setConcertPosterFiles(QList<DataFile*> files);
    void setConcertFanartFiles(QList<DataFile*> files);
    void setMediaCenterInterface(int interface);
    void setUseCache(bool useCache);

public slots:
    void saveSettings();

private:
    static Settings *m_instance;
    QSettings m_settings;

    QList<SettingsDir> m_movieDirectories;
    QList<SettingsDir> m_tvShowDirectories;
    QList<SettingsDir> m_concertDirectories;
    QSize m_mainWindowSize;
    QPoint m_mainWindowPosition;
    QByteArray m_mainSplitterState;
    int m_mediaCenterInterface;
    QString m_xbmcMysqlHost;
    QString m_xbmcMysqlDatabase;
    QString m_xbmcMysqlUser;
    QString m_xbmcMysqlPassword;
    QString m_xbmcSqliteDatabase;
    QString m_xbmcThumbnailPath;
    bool m_debugModeActivated;
    QString m_debugLogPath;
    bool m_youtubePluginUrls;
    QList<DataFile*> m_movieNfoFiles;
    QList<DataFile*> m_moviePosterFiles;
    QList<DataFile*> m_movieFanartFiles;
    QList<DataFile*> m_concertNfoFiles;
    QList<DataFile*> m_concertPosterFiles;
    QList<DataFile*> m_concertFanartFiles;
    QList<DataFile*> m_tvShowPosterFiles;
    QList<DataFile*> m_tvShowBannerFiles;
    bool m_useCache;

};

#endif // SETTINGS_H
