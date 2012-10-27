#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QPoint>
#include <QSettings>
#include <QSize>

#include "globals/Globals.h"
#include "settings/DataFile.h"

/**
 * @brief The Settings class
 */
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
    QString excludeWords();
    int mediaCenterInterface();
    QString xbmcMysqlHost();
    QString xbmcMysqlDatabase();
    QString xbmcMysqlUser();
    QString xbmcMysqlPassword();
    QString xbmcSqliteDatabase();
    QString xbmcThumbnailPath();
    bool useProxy();
    int proxyType();
    QString proxyHost();
    int proxyPort();
    QString proxyUsername();
    QString proxyPassword();
    bool debugModeActivated();
    QString debugLogPath();
    bool useYoutubePluginUrls();
    QList<DataFile*> movieNfoFiles();
    QList<DataFile*> moviePosterFiles();
    QList<DataFile*> movieFanartFiles();
    QList<DataFile*> movieCdArtFiles(bool onlyEnabled = false);
    QList<DataFile*> movieLogoFiles(bool onlyEnabled = false);
    QList<DataFile*> movieClearArtFiles(bool onlyEnabled = false);
    QList<DataFile*> tvShowPosterFiles();
    QList<DataFile*> tvShowBannerFiles();
    QList<DataFile*> tvShowLogoFiles(bool onlyEnabled = false);
    QList<DataFile*> tvShowClearArtFiles(bool onlyEnabled = false);
    QList<DataFile*> tvShowCharacterArtFiles(bool onlyEnabled = false);
    QList<DataFile*> concertNfoFiles();
    QList<DataFile*> concertPosterFiles();
    QList<DataFile*> concertFanartFiles();
    QList<DataFile*> concertCdArtFiles(bool onlyEnabled = false);
    QList<DataFile*> concertLogoFiles(bool onlyEnabled = false);
    QList<DataFile*> concertClearArtFiles(bool onlyEnabled = false);
    QList<DataFile*> enabledMovieNfoFiles();
    QList<DataFile*> enabledMoviePosterFiles();
    QList<DataFile*> enabledMovieFanartFiles();
    QList<DataFile*> enabledTvShowPosterFiles();
    QList<DataFile*> enabledTvShowBannerFiles();
    QList<DataFile*> enabledConcertNfoFiles();
    QList<DataFile*> enabledConcertPosterFiles();
    QList<DataFile*> enabledConcertFanartFiles();
    bool useCache();
    bool autoLoadStreamDetails();

    void setMainWindowSize(QSize mainWindowSize);
    void setMainWindowPosition(QPoint mainWindowPosition);
    void setMainSplitterState(QByteArray state);
    void setMovieDirectories(QList<SettingsDir> dirs);
    void setTvShowDirectories(QList<SettingsDir> dirs);
    void setConcertDirectories(QList<SettingsDir> dirs);
    void setExcludeWords(QString words);
    void setXbmcMysqlHost(QString host);
    void setXbmcMysqlDatabase(QString db);
    void setXbmcMysqlUser(QString user);
    void setXbmcMysqlPassword(QString password);
    void setUseProxy(bool use);
    void setProxyType(int type);
    void setProxyHost(QString host);
    void setProxyPort(int port);
    void setProxyUsername(QString username);
    void setProxyPassword(QString password);
    void setUseYoutubePluginUrls(bool use);
    void setXbmcSqliteDatabase(QString file);
    void setXbmcThumbnailPath(QString path);
    void setDebugModeActivated(bool enabled);
    void setDebugLogPath(QString path);
    void setMovieNfoFiles(QList<DataFile*> files);
    void setMoviePosterFiles(QList<DataFile*> files);
    void setMovieFanartFiles(QList<DataFile*> files);
    void setMovieCdArtFiles(QList<DataFile*> files);
    void setMovieLogoFiles(QList<DataFile*> files);
    void setMovieClearArtFiles(QList<DataFile*> files);
    void setTvShowPosterFiles(QList<DataFile*> files);
    void setTvShowBannerFiles(QList<DataFile*> files);
    void setTvShowLogoFiles(QList<DataFile*> files);
    void setTvShowClearArtFiles(QList<DataFile*> files);
    void setTvShowCharacterArtFiles(QList<DataFile*> files);
    void setConcertNfoFiles(QList<DataFile*> files);
    void setConcertPosterFiles(QList<DataFile*> files);
    void setConcertFanartFiles(QList<DataFile*> files);
    void setConcertCdArtFiles(QList<DataFile*> files);
    void setConcertLogoFiles(QList<DataFile*> files);
    void setConcertClearArtFiles(QList<DataFile*> files);
    void setMediaCenterInterface(int interface);
    void setUseCache(bool useCache);
    void setAutoLoadStreamDetails(bool autoLoad);

public slots:
    void saveSettings();

private:
    static Settings *m_instance;
    QSettings m_settings;

    QList<SettingsDir> m_movieDirectories;
    QList<SettingsDir> m_tvShowDirectories;
    QList<SettingsDir> m_concertDirectories;
    QString m_excludeWords;
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
    bool m_useProxy;
    int m_proxyType;
    QString m_proxyHost;
    int m_proxyPort;
    QString m_proxyUsername;
    QString m_proxyPassword;
    bool m_debugModeActivated;
    QString m_debugLogPath;
    bool m_youtubePluginUrls;
    QList<DataFile*> m_movieNfoFiles;
    QList<DataFile*> m_moviePosterFiles;
    QList<DataFile*> m_movieFanartFiles;
    QList<DataFile*> m_movieCdArtFiles;
    QList<DataFile*> m_movieLogoFiles;
    QList<DataFile*> m_movieClearArtFiles;
    QList<DataFile*> m_concertNfoFiles;
    QList<DataFile*> m_concertPosterFiles;
    QList<DataFile*> m_concertFanartFiles;
    QList<DataFile*> m_concertCdArtFiles;
    QList<DataFile*> m_concertLogoFiles;
    QList<DataFile*> m_concertClearArtFiles;
    QList<DataFile*> m_tvShowPosterFiles;
    QList<DataFile*> m_tvShowBannerFiles;
    QList<DataFile*> m_tvShowLogoFiles;
    QList<DataFile*> m_tvShowClearArtFiles;
    QList<DataFile*> m_tvShowCharacterArtFiles;
    bool m_useCache;
    bool m_autoLoadStreamDetails;

    void setupProxy();
};

#endif // SETTINGS_H
