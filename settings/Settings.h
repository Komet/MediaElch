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
    bool mainWindowMaximized();
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
    QList<DataFile> dataFiles(int type);
    bool usePlotForOutline();

    bool autoLoadStreamDetails();

    void setMainWindowSize(QSize mainWindowSize);
    void setMainWindowPosition(QPoint mainWindowPosition);
    void setMainWindowMaximized(bool max);
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
    void setMediaCenterInterface(int interface);
    void setAutoLoadStreamDetails(bool autoLoad);
    void setDataFiles(QList<DataFile> files);
    void setUsePlotForOutline(bool use);

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
    bool m_mainWindowMaximized;
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
    bool m_autoLoadStreamDetails;
    QList<DataFile> m_dataFiles;
    QList<DataFile> m_initialDataFiles;
    bool m_usePlotForOutline;

    void setupProxy();
};

#endif // SETTINGS_H
