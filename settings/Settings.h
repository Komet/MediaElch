#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QPoint>
#include <QSettings>
#include <QSize>

#include "globals/Globals.h"
#include "settings/AdvancedSettings.h"
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
    AdvancedSettings* advanced();
    void loadSettings();
    void loadSettings(QSettings &settings);
    QSettings *settings();

    QSize mainWindowSize();
    QPoint mainWindowPosition();
    QSize settingsWindowSize();
    QPoint settingsWindowPosition();
    QSize importDialogSize();
    QPoint importDialogPosition();
    bool mainWindowMaximized();
    QByteArray mainSplitterState();
    QList<SettingsDir> movieDirectories();
    QList<SettingsDir> tvShowDirectories();
    QList<SettingsDir> concertDirectories();
    QStringList downloadDirectories();
    QString unrar();
    bool deleteArchives();
    QString excludeWords();
    QString xbmcHost();
    int xbmcPort();
    bool useProxy();
    int proxyType();
    QString proxyHost();
    int proxyPort();
    QString proxyUsername();
    QString proxyPassword();
    bool debugModeActivated();
    QString debugLogPath();
    bool useYoutubePluginUrls();
    bool downloadActorImages();
    QList<DataFile> dataFiles(int type);
    QList<DataFile> dataFilesFrodo(int type = -1);
    bool usePlotForOutline();
    QList<int> scraperInfos(MainWidgets widget, QString scraperId);
    void renamePatterns(int renameType, QString &fileNamePattern, QString &fileNamePatternMulti, QString &directoryPattern, QString &seasonPattern);
    void renamings(int renameType, bool &files, bool &folders, bool &seasonDirectories);
    int tvShowUpdateOption();
    bool ignoreArticlesWhenSorting() const;
    MovieSetArtworkType movieSetArtworkType() const;
    QString movieSetArtworkDirectory() const;
    QList<MediaStatusColumns> mediaStatusColumns() const;
    bool tvShowDvdOrder() const;
    bool dontShowDeleteImageConfirm() const;
    QMap<int, QString> customMovieScraper() const;
    int currentMovieScraper() const;
    bool keepDownloadSource() const;
    bool checkForUpdates() const;
    bool showMissingEpisodesHint() const;
    bool multiScrapeOnlyWithId() const;
    bool multiScrapeSaveEach() const;

    bool autoLoadStreamDetails();

    void setMainWindowSize(QSize mainWindowSize);
    void setMainWindowPosition(QPoint mainWindowPosition);
    void setSettingsWindowSize(QSize settingsWindowSize);
    void setSettingsWindowPosition(QPoint settingsWindowPosition);
    void setImportDialogSize(QSize size);
    void setImportDialogPosition(QPoint position);
    void setMainWindowMaximized(bool max);
    void setMainSplitterState(QByteArray state);
    void setMovieDirectories(QList<SettingsDir> dirs);
    void setTvShowDirectories(QList<SettingsDir> dirs);
    void setConcertDirectories(QList<SettingsDir> dirs);
    void setDownloadDirectories(QStringList dirs);
    void setUnrar(QString unrar);
    void setDeleteArchives(bool deleteArchives);
    void setExcludeWords(QString words);
    void setUseProxy(bool use);
    void setProxyType(int type);
    void setProxyHost(QString host);
    void setProxyPort(int port);
    void setProxyUsername(QString username);
    void setProxyPassword(QString password);
    void setUseYoutubePluginUrls(bool use);
    void setDownloadActorImages(bool download);
    void setDebugModeActivated(bool enabled);
    void setDebugLogPath(QString path);
    void setAutoLoadStreamDetails(bool autoLoad);
    void setDataFiles(QList<DataFile> files);
    void setUsePlotForOutline(bool use);
    void setXbmcHost(QString host);
    void setXbmcPort(int port);
    void setScraperInfos(MainWidgets widget, QString scraperNo, QList<int> items);
    void setRenamePatterns(int renameType, QString fileNamePattern, QString fileNamePatternMulti, QString directoryPattern, QString seasonPattern);
    void setRenamings(int renameType, bool files, bool folders, bool seasonDirectories);
    void setTvShowUpdateOption(int option);
    void setIgnoreArticlesWhenSorting(bool ignore);
    void setMovieSetArtworkType(MovieSetArtworkType type);
    void setMovieSetArtworkDirectory(QString dir);
    void setMediaStatusColumns(QList<MediaStatusColumns> columns);
    void setTvShowDvdOrder(bool order);
    void setDontShowDeleteImageConfirm(bool show);
    void setCustomMovieScraper(QMap<int, QString> customMovieScraper);
    void setCurrentMovieScraper(int current);
    void setKeepDownloadSource(bool keep);
    void setCheckForUpdates(bool check);
    void setShowMissingEpisodesHint(bool show);
    void setMultiScrapeOnlyWithId(bool onlyWithId);
    void setMultiScrapeSaveEach(bool saveEach);

public slots:
    void saveSettings();

private:
    static Settings *m_instance;
    QSettings m_settings;
    AdvancedSettings *m_advancedSettings;

    QList<SettingsDir> m_movieDirectories;
    QList<SettingsDir> m_tvShowDirectories;
    QList<SettingsDir> m_concertDirectories;
    QStringList m_downloadDirectories;
    QString m_unrar;
    bool m_deleteArchives;
    QString m_excludeWords;
    QSize m_mainWindowSize;
    QPoint m_mainWindowPosition;
    QSize m_settingsWindowSize;
    QPoint m_settingsWindowPosition;
    QSize m_importDialogSize;
    QPoint m_importDialogPosition;
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
    bool m_downloadActorImages;
    bool m_autoLoadStreamDetails;
    QList<DataFile> m_dataFiles;
    QList<DataFile> m_initialDataFilesFrodo;
    bool m_usePlotForOutline;
    QString m_xbmcHost;
    int m_xbmcPort;
    bool m_ignoreArticlesWhenSorting;
    int m_movieSetArtworkType;
    QString m_movieSetArtworkDirectory;
    QList<MediaStatusColumns> m_mediaStatusColumns;
    bool m_tvShowDvdOrder;
    bool m_dontShowDeleteImageConfirm;
    QMap<int, QString> m_customMovieScraper;
    int m_currentMovieScraper;
    bool m_keepDownloadSource;
    bool m_checkForUpdates;
    bool m_showMissingEpisodesHint;
    bool m_multiScrapeOnlyWithId;
    bool m_multiScrapeSaveEach;

    void setupProxy();
};

#endif // SETTINGS_H
