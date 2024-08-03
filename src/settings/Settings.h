#pragma once

#include "data/tv_show/SeasonOrder.h"
#include "globals/Globals.h"
#include "model/MovieModel.h"
#include "settings/AdvancedSettings.h"
#include "settings/DataFile.h"
#include "settings/DirectorySettings.h"
#include "settings/KodiSettings.h"
#include "settings/NetworkSettings.h"
#include "ui/renamer/RenamerDialog.h"

#include <QHash>
#include <QMap>
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
protected:
    explicit Settings(QObject* parent);

public:
    static Settings* instance(QObject* parent = nullptr);
    AdvancedSettings* advanced();
    void loadSettings();
    QSettings* settings();

public:
    using Value = QVariant;

    struct Key
    {
        QString moduleName;
        QString key;

        Key() = default;
        Key(QString _moduleName, QString _key) : moduleName{_moduleName}, key{_key} {}

        ELCH_NODISCARD bool isNull() const { return key.isEmpty(); };
        ELCH_NODISCARD bool operator==(const Key& k) const { return k.key == key; };
        ELCH_NODISCARD bool operator<(const Key& k) const { return k.key < key; };
    };

    struct Item
    {
        Key key;
        Value value;
        Value defaultValue;
        QString description;

        bool isNull() const { return key.isNull(); }
    };

    using Items = QMap<Key, Item>;
    using Actions = QMap<Key, QVector<QPair<QObject*, std::function<void()>>>>;

    /// Get the value under the given key.
    /// Use in type-safe interfaces built on top of Settings.
    ELCH_NODISCARD virtual Value value(const Key& key);
    virtual void setValue(const Key& key, const Value& value);
    virtual void setDefaultValue(const Key& key, const Value& value);

    const Items& items() const;

    void onSettingChanged(Key key, QObject* context, std::function<void()> callback);

    void beginTransaction();
    void commitTransaction();
    void abortTransaction();

private:
    ELCH_NODISCARD Item& findItem(const Key& key);
    void addItem(Item item);
    void emitChangeFor(const Settings::Key& key) const;
    void writeValueToDisk(const Key& key, const Value& value);
    void readAllItemsFromDisk();

    // TODO: Move these settings into custom ones
public:
    QSize importDialogSize();
    QPoint importDialogPosition();
    QSize makeMkvDialogSize();
    QPoint makeMkvDialogPosition();
    QByteArray movieDuplicatesSplitterState();

    DirectorySettings& directorySettings();
    KodiSettings& kodiSettings();
    NetworkSettings& networkSettings();

    QStringList excludeWords();
    bool useYoutubePluginUrls() const;
    bool downloadActorImages() const;
    QVector<DataFile> dataFiles(DataFileType dataType);
    QVector<DataFile> dataFiles(ImageType dataType);
    QVector<DataFile> dataFilesFrodo(DataFileType type = DataFileType::NoType);
    bool usePlotForOutline() const;
    bool ignoreDuplicateOriginalTitle() const;
    void renamePatterns(RenameType renameType,
        QString& fileNamePattern,
        QString& fileNamePatternMulti,
        QString& directoryPattern,
        QString& seasonPattern);
    void renamings(RenameType renameType, bool& files, bool& folders, bool& seasonDirectories);

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
    QString theme();
    mediaelch::DirectoryPath lastImagePath();

    template<typename T>
    QSet<T> scraperInfos(QString scraperId); // TODO

    QSet<ConcertScraperInfo> scraperInfosConcert(const QString& scraperId);
    QSet<ShowScraperInfo> scraperInfosShow(const QString& scraperId);
    QSet<EpisodeScraperInfo> scraperInfosEpisode(const QString& scraperId);

    bool autoLoadStreamDetails() const;

    void setImportDialogSize(QSize size);
    void setImportDialogPosition(QPoint position);
    void setMakeMkvDialogSize(QSize size);
    void setMakeMkvDialogPosition(QPoint position);
    void setMovieDuplicatesSplitterState(QByteArray state);

    void setExcludeWords(QString words);
    void setUseYoutubePluginUrls(bool use);
    void setDownloadActorImages(bool download);
    void setAutoLoadStreamDetails(bool autoLoad);
    void setDataFiles(QVector<DataFile> files);
    void setUsePlotForOutline(bool use);
    void setIgnoreDuplicateOriginalTitle(bool ignoreDuplicateOriginalTitle);
    void setScraperInfos(const QString& scraperNo, const QSet<MovieScraperInfo>& items);
    void setScraperInfosShow(const QString& scraperId, const QSet<ShowScraperInfo>& items);
    void setScraperInfosEpisode(const QString& scraperId, const QSet<EpisodeScraperInfo>& items);
    void setScraperInfosConcert(const QString& scraperId, const QSet<ConcertScraperInfo>& items);
    void setScraperInfos(const QString& scraperNo, const QSet<MusicScraperInfo>& items);
    void setRenamePatterns(RenameType renameType,
        QString fileNamePattern,
        QString fileNamePatternMulti,
        QString directoryPattern,
        QString seasonPattern);
    void setRenamings(RenameType renameType, bool files, bool folders, bool seasonDirectories);
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
    void setTheme(QString theme);
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
    NetworkSettings m_networkSettings;

    QStringList m_excludeWords;
    QSize m_importDialogSize;
    QPoint m_importDialogPosition;
    QSize m_makeMkvDialogSize;
    QPoint m_makeMkvDialogPosition;
    QByteArray m_movieDuplicatesSplitterState;
    bool m_youtubePluginUrls = false;
    bool m_downloadActorImages = false;
    bool m_autoLoadStreamDetails = false;

    QVector<DataFile> m_dataFiles;
    QVector<DataFile> m_initialDataFilesFrodo;
    bool m_usePlotForOutline = false;
    bool m_ignoreDuplicateOriginalTitle = true;
    bool m_ignoreArticlesWhenSorting = false;
    MovieSetArtworkType m_movieSetArtworkType = MovieSetArtworkType::ArtworkNextToMovies;
    mediaelch::DirectoryPath m_movieSetArtworkDirectory;
    QVector<MediaStatusColumn> m_mediaStatusColumns;
    SeasonOrder m_seasonOrder = SeasonOrder::Aired;
    bool m_dontShowDeleteImageConfirm = false;
    QMap<MovieScraperInfo, QString> m_customMovieScraper;
    QMap<ShowScraperInfo, QString> m_customTvScraperShow;
    QMap<EpisodeScraperInfo, QString> m_customTvScraperEpisode;
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
    QString m_theme;
    bool m_donated = false;
    mediaelch::DirectoryPath m_lastImagePath;
    int m_extraFanartsMusicArtists = 0;

private:
    Items m_items;
    Items m_localItems;
    Actions m_callbacks;
    bool m_isTransactionInProgress{false};
};
