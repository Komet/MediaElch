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
    QString xbmcHost();
    int xbmcPort();
    QString xbmcUsername();
    QString xbmcPassword();
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
    void setUseProxy(bool use);
    void setProxyType(int type);
    void setProxyHost(QString host);
    void setProxyPort(int port);
    void setProxyUsername(QString username);
    void setProxyPassword(QString password);
    void setUseYoutubePluginUrls(bool use);
    void setDebugModeActivated(bool enabled);
    void setDebugLogPath(QString path);
    void setAutoLoadStreamDetails(bool autoLoad);
    void setDataFiles(QList<DataFile> files);
    void setUsePlotForOutline(bool use);
    void setXbmcHost(QString host);
    void setXbmcPort(int port);
    void setXbmcUsername(QString username);
    void setXbmcPassword(QString password);

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
    QString m_xbmcHost;
    int m_xbmcPort;
    QString m_xbmcUsername;
    QString m_xbmcPassword;

    void setupProxy();
};

#endif // SETTINGS_H
