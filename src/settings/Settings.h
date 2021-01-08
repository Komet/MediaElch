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
#include "tv_shows/SeasonOrder.h"

#include <QHash>
#include <QObject>
#include <QPoint>
#include <QSettings>
#include <QSize>

#include <memory>
#include <string>
#include <unordered_map>

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
    ScraperSettings* scraperSettings(const QString& id);

    QSize mainWindowSize();
    QPoint mainWindowPosition();
    QSize settingsWindowSize();
    QPoint settingsWindowPosition();
    QSize importDialogSize();
    QPoint importDialogPosition();
    QSize makeMkvDialogSize();
    QPoint makeMkvDialogPosition();
    bool mainWindowMaximized() const;
    QByteArray mainSplitterState();
    QByteArray movieDuplicatesSplitterState();

    DirectorySettings& directorySettings();
    KodiSettings& kodiSettings();
    ImportSettings& importSettings();
    NetworkSettings& networkSettings();

    QString csvExportSeparator();
    QString csvExportReplacement();
    QStringList csvExportTypes();
    QStringList csvExportMovieFields();
    QStringList csvExportTvShowFields();
    QStringList csvExportTvEpisodeFields();
    QStringList csvExportConcertFields();
    QStringList csvExportMusicArtistFields();
    QStringList csvExportMusicAlbumFields();

    bool deleteArchives() const;
    QStringList excludeWords();
    bool debugModeActivated() const;
    bool useYoutubePluginUrls() const;
    bool downloadActorImages() const;
    QVector<DataFile> dataFiles(DataFileType dataType);
    QVector<DataFile> dataFiles(ImageType dataType);
    QVector<DataFile> dataFilesFrodo(DataFileType type = DataFileType::NoType);
    bool usePlotForOutline() const;
    bool ignoreDuplicateOriginalTitle() const;
    void renamePatterns(Renamer::RenameType renameType,
        QString& fileNamePattern,
        QString& fileNamePatternMulti,
        QString& directoryPattern,
        QString& seasonPattern);
    void renamings(Renamer::RenameType renameType, bool& files, bool& folders, bool& seasonDirectories);

    int tvShowUpdateOption();
    bool ignoreArticlesWhenSorting() const;
    SeasonOrder seasonOrder() const;

    MovieSetArtworkType movieSetArtworkType() const;
    mediaelch::DirectoryPath movieSetArtworkDirectory() const;

    QVector<MediaStatusColumn> mediaStatusColumns() const;
    bool dontShowDeleteImageConfirm() const;
    const QMap<MovieScraperInfo, QString>& customMovieScraper() const;
    const QMap<ShowScraperInfo, QString>& customTvScraperShow() const;
    const QMap<EpisodeScraperInfo, QString>& customTvScraperEpisode() const;
    int currentMovieScraper() const;
    const QString& currentTvShowScraper() const;
    const QString& currentConcertScraper() const;
    bool keepDownloadSource() const;
    bool checkForUpdates() const;
    bool showMissingEpisodesHint() const;
    bool multiScrapeOnlyWithId() const;
    bool multiScrapeSaveEach() const;
    mediaelch::DirectoryPath databaseDir();
    mediaelch::DirectoryPath imageCacheDir();
    mediaelch::DirectoryPath exportTemplatesDir();
    bool showAdultScrapers() const;
    QString startupSection();
    bool donated() const;
    mediaelch::DirectoryPath lastImagePath();

    template<typename T>
    QSet<T> scraperInfos(QString scraperId); // TODO

    QSet<ConcertScraperInfo> scraperInfosConcert(const QString& scraperId);
    QSet<ShowScraperInfo> scraperInfosShow(const QString& scraperId);
    QSet<EpisodeScraperInfo> scraperInfosEpisode(const QString& scraperId);

    bool autoLoadStreamDetails() const;

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

    void setCsvExportSeparator(QString separator);
    void setCsvExportReplacement(QString replacement);
    void setCsvExportTypes(QStringList types);
    void setCsvExportMovieFields(QStringList fields);
    void setCsvExportTvShowFields(QStringList fields);
    void setCsvExportTvEpisodeFields(QStringList fields);
    void setCsvExportConcertFields(QStringList fields);
    void setCsvExportMusicArtistFields(QStringList fields);
    void setCsvExportMusicAlbumFields(QStringList fields);

    void setDeleteArchives(bool deleteArchives);
    void setExcludeWords(QString words);
    void setUseYoutubePluginUrls(bool use);
    void setDownloadActorImages(bool download);
    void setDebugModeActivated(bool enabled);
    void setDebugLogPath(mediaelch::FilePath path);
    void setAutoLoadStreamDetails(bool autoLoad);
    void setDataFiles(QVector<DataFile> files);
    void setUsePlotForOutline(bool use);
    void setIgnoreDuplicateOriginalTitle(bool ignoreDuplicateOriginalTitle);
    void setScraperInfos(const QString& scraperNo, const QSet<MovieScraperInfo>& items);
    void setScraperInfosShow(const QString& scraperId, const QSet<ShowScraperInfo>& items);
    void setScraperInfosEpisode(const QString& scraperId, const QSet<EpisodeScraperInfo>& items);
    void setScraperInfosConcert(const QString& scraperId, const QSet<ConcertScraperInfo>& items);
    void setScraperInfos(const QString& scraperNo, const QSet<MusicScraperInfo>& items);
    void setRenamePatterns(Renamer::RenameType renameType,
        QString fileNamePattern,
        QString fileNamePatternMulti,
        QString directoryPattern,
        QString seasonPattern);
    void setRenamings(Renamer::RenameType renameType, bool files, bool folders, bool seasonDirectories);
    void setTvShowUpdateOption(int option);
    void setIgnoreArticlesWhenSorting(bool ignore);
    void setMovieSetArtworkType(MovieSetArtworkType type);
    void setMovieSetArtworkDirectory(mediaelch::DirectoryPath dir);
    void setMediaStatusColumn(QVector<MediaStatusColumn> columns);
    void setSeasonOrder(SeasonOrder order);
    void setDontShowDeleteImageConfirm(bool show);
    void setCustomMovieScraper(QMap<MovieScraperInfo, QString> customMovieScraper);
    void setCustomTvScraperShow(QMap<ShowScraperInfo, QString> customTvScraper);
    void setCustomTvScraperEpisode(QMap<EpisodeScraperInfo, QString> customTvScraper);
    void setCurrentTvShowScraper(const QString& current);
    void setCurrentMovieScraper(int current);
    void setCurrentConcertScraper(const QString& current);
    void setKeepDownloadSource(bool keep);
    void setCheckForUpdates(bool check);
    void setShowMissingEpisodesHint(bool show);
    void setMultiScrapeOnlyWithId(bool onlyWithId);
    void setMultiScrapeSaveEach(bool saveEach);
    void setShowAdultScrapers(bool show);
    void setStartupSection(QString startupSection);
    void setDonated(bool donated);
    void setLastImagePath(mediaelch::DirectoryPath path);

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
    QStringList m_excludeWords;
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
    bool m_youtubePluginUrls = false;
    bool m_downloadActorImages = false;
    bool m_autoLoadStreamDetails = false;

    QString m_csvExportSeparator;
    QString m_csvExportReplacement;
    QStringList m_csvExportTypes;
    QStringList m_csvExportMovieFields;
    QStringList m_csvExportTvShowFields;
    QStringList m_csvExportTvEpisodeFields;
    QStringList m_csvExportConcertFields;
    QStringList m_csvExportMusicArtistFields;
    QStringList m_csvExportMusicAlbumFields;

    QVector<DataFile> m_dataFiles;
    QVector<DataFile> m_initialDataFilesFrodo;
    bool m_usePlotForOutline = false;
    bool m_ignoreDuplicateOriginalTitle = true;
    bool m_ignoreArticlesWhenSorting = false;
    MovieSetArtworkType m_movieSetArtworkType = MovieSetArtworkType::SingleSetFolder;
    mediaelch::DirectoryPath m_movieSetArtworkDirectory;
    QVector<MediaStatusColumn> m_mediaStatusColumns;
    SeasonOrder m_seasonOrder = SeasonOrder::Aired;
    bool m_dontShowDeleteImageConfirm = false;
    QMap<MovieScraperInfo, QString> m_customMovieScraper;
    QMap<ShowScraperInfo, QString> m_customTvScraperShow;
    QMap<EpisodeScraperInfo, QString> m_customTvScraperEpisode;
    std::unordered_map<std::string, std::unique_ptr<ScraperSettings>> m_scraperSettings;
    int m_currentMovieScraper = 0;
    QString m_currentTvShowScraper;
    QString m_currentConcertScraper;
    bool m_keepDownloadSource = false;
    bool m_checkForUpdates = false;
    bool m_showMissingEpisodesHint = false;
    bool m_multiScrapeOnlyWithId = false;
    bool m_multiScrapeSaveEach = false;
    bool m_showAdultScrapers = false;
    QString m_startupSection;
    bool m_donated = false;
    mediaelch::DirectoryPath m_lastImagePath;
    int m_extraFanartsMusicArtists = 0;

    QPoint fixWindowPosition(QPoint p);
};
