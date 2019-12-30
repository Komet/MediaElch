#pragma once

#include "globals/Globals.h"
#include "renamer/RenamerDialog.h"
#include "settings/AdvancedSettings.h"
#include "settings/DataFile.h"
#include "settings/DirectorySettings.h"
#include "settings/ImportSettings.h"
#include "settings/KodiSettings.h"
#include "settings/NetworkSettings.h"
#include "settings/ScraperSettings.h"

#include <QObject>
#include <QPoint>
#include <QSettings>
#include <QSize>

#include <memory>

class Settings : public QObject
{
    Q_OBJECT
private:
    explicit Settings(QObject* parent);

public:
    static Settings* instance(QObject* parent = nullptr);
    AdvancedSettings* advanced();
    void loadSettings();
    QSettings* settings();
    ScraperSettings& scraperSettings();

    QSize mainWindowSize();
    QPoint mainWindowPosition();
    QSize settingsWindowSize();
    QPoint settingsWindowPosition();
    QSize importDialogSize();
    QPoint importDialogPosition();
    QSize makeMkvDialogSize();
    QPoint makeMkvDialogPosition();
    bool mainWindowMaximized();
    QByteArray mainSplitterState();
    QByteArray movieDuplicatesSplitterState();

    DirectorySettings& directorySettings();
    KodiSettings& kodiSettings();
    ImportSettings& importSettings();
    NetworkSettings& networkSettings();

    bool deleteArchives();
    QString excludeWords();
    bool debugModeActivated();
    QString debugLogPath();
    bool useYoutubePluginUrls();
    bool downloadActorImages();
    QVector<DataFile> dataFiles(DataFileType dataType);
    QVector<DataFile> dataFiles(ImageType dataType);
    QVector<DataFile> dataFilesFrodo(DataFileType type = DataFileType::NoType);
    bool usePlotForOutline();
    template<typename T>
    QVector<T> scraperInfos(MainWidgets widget, QString scraperId);
    void renamePatterns(Renamer::RenameType renameType,
        QString& fileNamePattern,
        QString& fileNamePatternMulti,
        QString& directoryPattern,
        QString& seasonPattern);
    void renamings(Renamer::RenameType renameType, bool& files, bool& folders, bool& seasonDirectories);
    int tvShowUpdateOption();
    bool ignoreArticlesWhenSorting() const;
    MovieSetArtworkType movieSetArtworkType() const;
    QString movieSetArtworkDirectory() const;
    QVector<MediaStatusColumn> mediaStatusColumns() const;
    bool tvShowDvdOrder() const;
    bool dontShowDeleteImageConfirm() const;
    QMap<MovieScraperInfos, QString> customMovieScraper() const;
    QMap<TvShowScraperInfos, QString> customTvScraper() const;
    int currentMovieScraper() const;
    bool keepDownloadSource() const;
    bool checkForUpdates() const;
    bool showMissingEpisodesHint() const;
    bool multiScrapeOnlyWithId() const;
    bool multiScrapeSaveEach() const;
    QString databaseDir();
    QString imageCacheDir();
    QString exportTemplatesDir();
    bool showAdultScrapers() const;
    QString startupSection();
    bool donated() const;
    QString lastImagePath();

    template<typename T>
    QVector<T> scraperInfos(QString scraperId);

    bool autoLoadStreamDetails();

    void setMainWindowSize(QSize mainWindowSize);
    void setMainWindowPosition(QPoint mainWindowPosition);
    void setSettingsWindowSize(QSize settingsWindowSize);
    void setSettingsWindowPosition(QPoint settingsWindowPosition);
    void setImportDialogSize(QSize size);
    void setImportDialogPosition(QPoint position);
    void setMakeMkvDialogSize(QSize size);
    void setMakeMkvDialogPosition(QPoint position);
    void setMainWindowMaximized(bool max);
    void setMainSplitterState(QByteArray state);
    void setMovieDuplicatesSplitterState(QByteArray state);
    void setDeleteArchives(bool deleteArchives);
    void setExcludeWords(QString words);
    void setUseYoutubePluginUrls(bool use);
    void setDownloadActorImages(bool download);
    void setDebugModeActivated(bool enabled);
    void setDebugLogPath(QString path);
    void setAutoLoadStreamDetails(bool autoLoad);
    void setDataFiles(QVector<DataFile> files);
    void setUsePlotForOutline(bool use);
    void setScraperInfos(MainWidgets widget, QString scraperNo, QVector<MovieScraperInfos> items);
    void setScraperInfos(MainWidgets widget, QString scraperNo, QVector<TvShowScraperInfos> items);
    void setScraperInfos(MainWidgets widget, QString scraperNo, QVector<ConcertScraperInfos> items);
    void setScraperInfos(MainWidgets widget, QString scraperNo, QVector<MusicScraperInfos> items);
    void setRenamePatterns(Renamer::RenameType renameType,
        QString fileNamePattern,
        QString fileNamePatternMulti,
        QString directoryPattern,
        QString seasonPattern);
    void setRenamings(Renamer::RenameType renameType, bool files, bool folders, bool seasonDirectories);
    void setTvShowUpdateOption(int option);
    void setIgnoreArticlesWhenSorting(bool ignore);
    void setMovieSetArtworkType(MovieSetArtworkType type);
    void setMovieSetArtworkDirectory(QString dir);
    void setMediaStatusColumn(QVector<MediaStatusColumn> columns);
    void setTvShowDvdOrder(bool order);
    void setDontShowDeleteImageConfirm(bool show);
    void setCustomMovieScraper(QMap<MovieScraperInfos, QString> customMovieScraper);
    void setCustomTvScraper(QMap<TvShowScraperInfos, QString> customTvScraper);
    void setCurrentMovieScraper(int current);
    void setKeepDownloadSource(bool keep);
    void setCheckForUpdates(bool check);
    void setShowMissingEpisodesHint(bool show);
    void setMultiScrapeOnlyWithId(bool onlyWithId);
    void setMultiScrapeSaveEach(bool saveEach);
    void setShowAdultScrapers(bool show);
    void setStartupSection(QString startupSection);
    void setDonated(bool donated);
    void setLastImagePath(QString path);

    static QString applicationDir();

    int extraFanartsMusicArtists() const;
    void setExtraFanartsMusicArtists(int extraFanartsMusicArtists);

public slots:
    void saveSettings();

signals:
    void sigSettingsSaved();
    void sigDonated(bool);

private:
    QSettings* m_settings;
    AdvancedSettings m_advancedSettings;

    DirectorySettings m_directorySettings;
    KodiSettings m_kodiSettings;
    ImportSettings m_importSettings;
    NetworkSettings m_networkSettings;

    bool m_deleteArchives = false;
    QString m_excludeWords;
    QSize m_mainWindowSize;
    QPoint m_mainWindowPosition;
    QSize m_settingsWindowSize;
    QPoint m_settingsWindowPosition;
    QSize m_importDialogSize;
    QPoint m_importDialogPosition;
    QSize m_makeMkvDialogSize;
    QPoint m_makeMkvDialogPosition;
    bool m_mainWindowMaximized = false;
    QByteArray m_mainSplitterState;
    QByteArray m_movieDuplicatesSplitterState;
    bool m_debugModeActivated = false;
    QString m_debugLogPath;
    bool m_youtubePluginUrls = false;
    bool m_downloadActorImages = false;
    bool m_autoLoadStreamDetails = false;
    QVector<DataFile> m_dataFiles;
    QVector<DataFile> m_initialDataFilesFrodo;
    bool m_usePlotForOutline = false;
    bool m_ignoreArticlesWhenSorting = false;
    MovieSetArtworkType m_movieSetArtworkType = MovieSetArtworkType::SingleSetFolder;
    QString m_movieSetArtworkDirectory;
    QVector<MediaStatusColumn> m_mediaStatusColumns;
    bool m_tvShowDvdOrder = false;
    bool m_dontShowDeleteImageConfirm = false;
    QMap<MovieScraperInfos, QString> m_customMovieScraper;
    QMap<TvShowScraperInfos, QString> m_customTvScraper;
    int m_currentMovieScraper = 0;
    bool m_keepDownloadSource = false;
    bool m_checkForUpdates = false;
    bool m_showMissingEpisodesHint = false;
    bool m_multiScrapeOnlyWithId = false;
    bool m_multiScrapeSaveEach = false;
    bool m_showAdultScrapers = false;
    QString m_startupSection;
    bool m_donated = false;
    QString m_lastImagePath;
    int m_extraFanartsMusicArtists = 0;

    QPoint fixWindowPosition(QPoint p);
};
