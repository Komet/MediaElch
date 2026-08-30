#include "Settings.h"

#include "globals/Manager.h"
#include "log/Log.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/music/MusicScraper.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "settings/AdvancedSettingsXmlReader.h"
#include "ui/renamer/RenamerDialog.h"

#include <QApplication>
#include <QDesktopServices>
#include <QStandardPaths>

// Keys for array/structured settings that cannot use the Key/Value registry
static constexpr char KEY_ALL_DATA_FILES[] = "AllDataFiles";
static constexpr char KEY_CUSTOM_MOVIE_SCRAPER[] = "CustomMovieScraper";
static constexpr char KEY_CUSTOM_TV_SCRAPER_SHOW[] = "CustomTvScraperShow";
static constexpr char KEY_CUSTOM_TV_SCRAPER_EPISODE[] = "CustomTvScraperEpisode";
static constexpr char KEY_MEDIA_STATUS_COLUMN[] = "MediaStatusColumn";
static constexpr char KEY_MOVIE_DUPLICATES_SPLITTER_STATE[] = "MovieDuplicatesSplitterState";
static constexpr char KEY_DOWNLOADS_IMPORT_DIALOG_POSITION[] = "Downloads/ImportDialogPosition";
static constexpr char KEY_DOWNLOADS_IMPORT_DIALOG_SIZE[] = "Downloads/ImportDialogSize";
static constexpr char KEY_DOWNLOADS_MAKE_MKV_DIALOG_POSITION[] = "Downloads/MakeMkvDialogPosition";
static constexpr char KEY_DOWNLOADS_MAKE_MKV_DIALOG_SIZE[] = "Downloads/MakeMkvDialogSize";

static constexpr char KEY_SCRAPER_TV_SHOW_DETAILS[] = "Scrapers/TvShows/%1";
static constexpr char KEY_SCRAPER_TV_EPISODE_DETAILS[] = "Scrapers/Episodes/%1";
static constexpr char KEY_SCRAPER_CONCERT_DETAILS[] = "Scrapers/Concerts/%1";

// Keys for scalar settings
static const Settings::Key KEY_AUTO_LOAD_STREAM_DETAILS{"globals", "AutoLoadStreamDetails"};
static const Settings::Key KEY_CHECK_FOR_UPDATES{"globals", "CheckForUpdates"};
static const Settings::Key KEY_DONATED{"globals", "Donated"};
static const Settings::Key KEY_THEME{"globals", "Theme"};
static const Settings::Key KEY_DOWNLOAD_ACTOR_IMAGES{"globals", "DownloadActorImages"};
static const Settings::Key KEY_DOWNLOADS_KEEP_SOURCE{"downloads", "Downloads/KeepSource"};
static const Settings::Key KEY_EXCLUDE_WORDS{"globals", "excludeWords"};
static const Settings::Key KEY_IGNORE_ARTICLES_WHEN_SORTING{"globals", "IgnoreArticlesWhenSorting"};
static const Settings::Key KEY_LAST_IMAGE_PATH{"globals", "LastImagePath"};
static const Settings::Key KEY_MOVIE_IGNORE_DUPLICATE_ORIGINAL_TITLE{"movies", "Movies/IgnoreDuplicateOriginalTitle"};
static const Settings::Key KEY_MOVIE_SET_ARTWORK_DIRECTORY{"movies", "MovieSetArtwork/Directory"};
static const Settings::Key KEY_MOVIE_SET_ARTWORK_STORING_TYPE{"movies", "MovieSetArtwork/StoringType"};
static const Settings::Key KEY_MOVIES_MULTI_SCRAPE_ONLY_WITH_ID{"movies", "Movies/MultiScrapeOnlyWithId"};
static const Settings::Key KEY_MOVIES_MULTI_SCRAPE_SAVE_EACH{"movies", "Movies/MultiScrapeSaveEach"};
static const Settings::Key KEY_MUSIC_ARTISTS_EXTRA_FANARTS{"music", "Music/Artists/ExtraFanarts"};
static const Settings::Key KEY_SCRAPER_CURRENT_MOVIE_SCRAPER{"scrapers", "Scraper/CurrentMovieScraper"};
static const Settings::Key KEY_SCRAPER_CURRENT_TV_SHOW_SCRAPER{"scrapers", "Scraper/CurrentTvShowScraper"};
static const Settings::Key KEY_SCRAPER_CURRENT_CONCERT_SCRAPER{"scrapers", "Scraper/CurrentConcertScraper"};
static const Settings::Key KEY_SCRAPERS_SHOW_ADULT{"scrapers", "Scrapers/ShowAdult"};
static const Settings::Key KEY_STARTUP_SECTION{"globals", "StartupSection"};
static const Settings::Key KEY_TV_SHOW_UPDATE_OPTION{"tv_shows", "TvShowUpdateOption"};
static const Settings::Key KEY_TV_SHOWS_SEASON_ORDER{"tv_shows", "TvShows/SeasonOrder"};
static const Settings::Key KEY_TV_SHOWS_SHOW_MISSING_EPISODES{"tv_shows", "TvShows/ShowMissingEpisodesHint"};
static const Settings::Key KEY_USE_PLOT_FOR_OUTLINE{"movies", "Movies/UsePlotForOutline"};
static const Settings::Key KEY_USE_YOUTUBE_PLUGIN_URL{"globals", "UseYoutubePluginURLs"};
static const Settings::Key KEY_WARNINGS_DO_NOT_SHOW_DELETE_IMAGE_CONFIRM{"warnings",
    "Warnings/DontShowDeleteImageConfirm"};

namespace {

QPoint fixWindowPosition(QPoint p)
{
    p.setX(qMax(0, p.x()));
    p.setY(qMax(0, p.y()));
    return p;
}


} // namespace

Settings::Settings(QObject* parent) : QObject(parent)
{
    auto advancedSettingsPair = AdvancedSettingsXmlReader::loadFromDefaultPath();
    m_advancedSettings = std::move(advancedSettingsPair.first);

    qCDebug(generic) << m_advancedSettings;

    if (m_advancedSettings.portableMode()) {
        qCDebug(generic) << "[Windows] Using portable mode!";
        m_settings = new QSettings(Settings::applicationDir() + "/MediaElch.ini", QSettings::IniFormat, this);
    } else {
        m_settings = new QSettings(this);
    }

    m_directorySettings.setQSettings(m_settings);
    m_networkSettings.setQSettings(m_settings);

    // Frodo
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MoviePoster, "<baseFileName>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieCdArt, "<baseFileName>-discart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieClearArt, "<baseFileName>-clearart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieLogo, "<baseFileName>-clearlogo.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieBanner, "<baseFileName>-banner.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieThumb, "<baseFileName>-landscape.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieSetPoster, "<setName>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieSetBackdrop, "<setName>-fanart.jpg", 0));

    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowNfo, "tvshow.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowBackdrop, "fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowBanner, "banner.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowCharacterArt, "characterart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowClearArt, "clearart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowLogo, "clearlogo.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowThumb, "landscape.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowPoster, "poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowPoster, "season-all-poster.jpg", 1));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowSeasonPoster, "season<seasonNumber>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowSeasonBackdrop, "season<seasonNumber>-fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowSeasonBanner, "season<seasonNumber>-banner.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowSeasonThumb, "season<seasonNumber>-landscape.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowEpisodeNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowEpisodeThumb, "<baseFileName>-thumb.jpg", 0));

    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertPoster, "<baseFileName>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertCdArt, "<baseFileName>-discart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertClearArt, "<baseFileName>-clearart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertLogo, "<baseFileName>-clearlogo.png", 0));

    m_initialDataFilesFrodo.append(DataFile(DataFileType::ArtistFanart, "fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ArtistLogo, "clearlogo.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ArtistThumb, "thumb.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::AlbumCdArt, "discart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::AlbumThumb, "thumb.jpg", 0));
}

/**
 * \brief Returns an instance of the settings
 * \param parent Parent widget
 * \return Instance of Settings
 */
Settings* Settings::instance(QObject* parent)
{
    static auto* m_instance = new Settings(parent);
    return m_instance;
}

QSettings* Settings::settings()
{
    return m_settings;
}

Settings::Value Settings::value(const Key& key) const
{
    const Item& item = findItem(key);
    if (item.isNull()) {
        return QVariant{};
    } else {
        return item.value;
    }
}

void Settings::setDefaultValue(const Settings::Key& key, const Settings::Value& value)
{
    Item& item = findItem(key);
    if (item.isNull()) {
        Item newItem;
        newItem.key = key;
        newItem.value = settings()->value(key.key, value);
        newItem.defaultValue = value;
        newItem.description = ""; // TODO

        addItem(newItem);

    } else {
        item.key = key; // may override key's module
        item.defaultValue = value;
    }
}

void Settings::setValue(const Settings::Key& key, const Settings::Value& value)
{
    Item& item = findItem(key);

    if (!item.isNull() && item.value == value) {
        return;
    }

    if (!m_isTransactionInProgress) {
        writeValueToDisk(key, value);
    }

    if (item.isNull()) {
        qCDebug(generic) << "[Settings] setValue(): Found item without default entry:" << key.key;
        Item newItem;
        newItem.key = key;
        newItem.value = value;
        addItem(newItem);

    } else {
        item.value = value;
    }

    emitChangeFor(key);
}

void Settings::loadSettings()
{
    readAllItemsFromDisk();

    // Globals — register defaults via the Key/Value registry
    setDefaultValue(KEY_AUTO_LOAD_STREAM_DETAILS, true);
    setDefaultValue(KEY_USE_PLOT_FOR_OUTLINE, true);
    setDefaultValue(KEY_MOVIE_IGNORE_DUPLICATE_ORIGINAL_TITLE, true);
    setDefaultValue(KEY_DOWNLOAD_ACTOR_IMAGES, true);
    setDefaultValue(KEY_IGNORE_ARTICLES_WHEN_SORTING, false);
    setDefaultValue(KEY_CHECK_FOR_UPDATES, true);
    setDefaultValue(KEY_SCRAPERS_SHOW_ADULT, false);
    setDefaultValue(KEY_STARTUP_SECTION, QStringLiteral("movies"));
    setDefaultValue(KEY_DONATED, false);
    setDefaultValue(KEY_USE_YOUTUBE_PLUGIN_URL, false);
    setDefaultValue(KEY_LAST_IMAGE_PATH, QDir::homePath());

    const QString themeDefault = QStringLiteral("auto");
    setDefaultValue(KEY_THEME, themeDefault);
    const QString loadedTheme = value(KEY_THEME).toString();
    if (loadedTheme != QLatin1String("auto") && loadedTheme != QLatin1String("dark")
        && loadedTheme != QLatin1String("light")) {
        qCWarning(generic) << "[Settings] Unknown theme value:" << loadedTheme << "(available: light, dark)";
        setValue(KEY_THEME, themeDefault);
    }

    // Window positions (stay in m_* — not needed for testing)
    m_movieDuplicatesSplitterState = settings()->value(KEY_MOVIE_DUPLICATES_SPLITTER_STATE).toByteArray();
    m_importDialogPosition = fixWindowPosition(settings()->value(KEY_DOWNLOADS_IMPORT_DIALOG_POSITION).toPoint());
    m_makeMkvDialogPosition = fixWindowPosition(settings()->value(KEY_DOWNLOADS_MAKE_MKV_DIALOG_POSITION).toPoint());

    // Tv Shows
    setDefaultValue(KEY_TV_SHOWS_SEASON_ORDER, 1);
    setDefaultValue(KEY_TV_SHOW_UPDATE_OPTION, 0);
    setDefaultValue(KEY_TV_SHOWS_SHOW_MISSING_EPISODES, true);

    // Warnings
    setDefaultValue(KEY_WARNINGS_DO_NOT_SHOW_DELETE_IMAGE_CONFIRM, false);

    m_directorySettings.loadSettings();
    m_networkSettings.loadSettings();

    // Exclude words — stored as comma-separated string; use Key/Value for the raw string
    static const QString excludeWordsDefault = QStringLiteral(
        "ac3,dts,ddp5.1,custom,dc,divx,divx5,dsr,dsrip,dutch,dvd,dvdrip,dvdscr,dvdscreener,screener,dvdivx,"
        "cam,fragment,fs,hdtv,hdrip,hdtvrip,internal,limited,"
        "multisubs,ntsc,ogg,ogm,pal,pdtv,proper,repack,rerip,retail,r3,r5,bd5,se,svcd,swedish,german,"
        "nfofix,unrated,ws,telesync,ts,telecine,tc,"
        "brrip,bdrip,480p,480i,576p,576i,720p,720i,1080p,1080i,2160p,"
        "hrhd,hrhdtv,hddvd,uhdtv,uhdv,bluray,uhd,"
        "x264,h264,h.264,h.265,h265,hevc,web-dl,"
        "xvid,xvidvd,xxx,www,mkv");
    // TODO: Add upgrade-process; currently new entries won't reach users with existing stored values.
    setDefaultValue(KEY_EXCLUDE_WORDS, excludeWordsDefault);

    // Scrapers
    setDefaultValue(KEY_SCRAPER_CURRENT_MOVIE_SCRAPER, 0);
    setDefaultValue(KEY_SCRAPER_CURRENT_TV_SHOW_SCRAPER, QString{});
    setDefaultValue(KEY_SCRAPER_CURRENT_CONCERT_SCRAPER, QString{});

    // Data Files
    QVector<DataFile> dataFiles;
    int dataFileSize = settings()->beginReadArray(KEY_ALL_DATA_FILES);
    for (int i = 0; i < dataFileSize; ++i) {
        settings()->setArrayIndex(i);
        DataFileType type = DataFileType(settings()->value("type").toInt());
        QString fileName = settings()->value("fileName").toString();
        if (fileName.isEmpty()) {
            for (const DataFile& initialDataFile : asConst(m_initialDataFilesFrodo)) {
                if (initialDataFile.type() == type) {
                    fileName = initialDataFile.fileName();
                    break;
                }
            }
        }
        int pos = settings()->value("pos").toInt();
        DataFile f(type, fileName, pos);
        dataFiles.append(f);
    }
    settings()->endArray();

    for (const DataFile& initialDataFile : asConst(m_initialDataFilesFrodo)) {
        bool found = false;
        for (const DataFile& df : dataFiles) {
            if (df.type() == initialDataFile.type()) {
                found = true;
                break;
            }
        }
        if (!found) {
            dataFiles << initialDataFile;
        }
    }

    if (dataFiles.isEmpty()) {
        m_dataFiles = m_initialDataFilesFrodo;
    } else {
        m_dataFiles = dataFiles;
    }

    // Movie set artwork
    setDefaultValue(KEY_MOVIE_SET_ARTWORK_STORING_TYPE, 0);
    setDefaultValue(KEY_MOVIE_SET_ARTWORK_DIRECTORY, QString{});

    // Media Status Columns (stays in m_* — list type)
    m_mediaStatusColumns.clear();
    for (const QVariant& column : settings()->value(KEY_MEDIA_STATUS_COLUMN).toList()) {
        m_mediaStatusColumns.append(static_cast<MediaStatusColumn>(column.toInt()));
    }

    m_customMovieScraper.clear();
    int customMovieScraperSize = settings()->beginReadArray(KEY_CUSTOM_MOVIE_SCRAPER);
    for (int i = 0; i < customMovieScraperSize; ++i) {
        settings()->setArrayIndex(i);
        m_customMovieScraper.insert(
            MovieScraperInfo(settings()->value("Info").toInt()), settings()->value("Scraper").toString());
    }
    settings()->endArray();

    // ------------------------------------------------------------------------

    m_customTvScraperShow.clear();
    const int customTvScraperShowSize = settings()->beginReadArray(KEY_CUSTOM_TV_SCRAPER_SHOW);
    for (int i = 0; i < customTvScraperShowSize; ++i) {
        settings()->setArrayIndex(i);
        m_customTvScraperShow.insert(
            ShowScraperInfo(settings()->value("Info").toInt()), settings()->value("Scraper").toString());
    }
    // Ensure that all details are set. Default is TmdbTv because it supports most (was TheTvDb previously).
    for (const ShowScraperInfo info : mediaelch::allShowScraperInfos()) {
        if (!m_customTvScraperShow.contains(info)) {
            m_customTvScraperShow.insert(info, mediaelch::scraper::TmdbTv::ID);
        }
    }
    settings()->endArray();

    m_customTvScraperEpisode.clear();
    const int customTvScraperEpisodeSize = settings()->beginReadArray(KEY_CUSTOM_TV_SCRAPER_EPISODE);
    for (int i = 0; i < customTvScraperEpisodeSize; ++i) {
        settings()->setArrayIndex(i);
        m_customTvScraperEpisode.insert(
            EpisodeScraperInfo(settings()->value("Info").toInt()), settings()->value("Scraper").toString());
    }
    // Ensure that all details are set. Default is TmdbTv because it supports most (was TheTvDb previously).
    const auto allEpisodeInfos = mediaelch::allEpisodeScraperInfos();
    for (const EpisodeScraperInfo info : allEpisodeInfos) {
        if (!m_customTvScraperEpisode.contains(info)) {
            m_customTvScraperEpisode.insert(info, mediaelch::scraper::TmdbTv::ID);
        }
    }
    settings()->endArray();

    // ------------------------------------------------------------------------

    // Downloads
    m_importDialogSize = settings()->value(KEY_DOWNLOADS_IMPORT_DIALOG_SIZE).toSize();
    m_makeMkvDialogSize = settings()->value(KEY_DOWNLOADS_MAKE_MKV_DIALOG_SIZE).toSize();
    setDefaultValue(KEY_DOWNLOADS_KEEP_SOURCE, true);

    // Movies
    setDefaultValue(KEY_MOVIES_MULTI_SCRAPE_ONLY_WITH_ID, false);
    setDefaultValue(KEY_MOVIES_MULTI_SCRAPE_SAVE_EACH, false);

    setDefaultValue(KEY_MUSIC_ARTISTS_EXTRA_FANARTS, 0);
}

void Settings::saveSettings()
{
    // All scalar Key/Value settings are written through setValue() as they change;
    // here we only need to flush anything that was mutated without going through setValue().
    // (None currently — this block is intentionally minimal.)

    m_directorySettings.saveSettings();
    m_networkSettings.saveSettings();

    settings()->beginWriteArray(KEY_ALL_DATA_FILES);
    for (int i = 0, n = qsizetype_to_int(m_dataFiles.count()); i < n; ++i) {
        settings()->setArrayIndex(i);
        settings()->setValue("type", static_cast<int>(m_dataFiles.at(i).type()));
        settings()->setValue("fileName", m_dataFiles.at(i).fileName());
        settings()->setValue("pos", m_dataFiles.at(i).pos());
    }
    settings()->endArray();

    QList<QVariant> columns;
    for (const MediaStatusColumn& column : asConst(m_mediaStatusColumns)) {
        columns.append(static_cast<int>(column));
    }
    settings()->setValue(KEY_MEDIA_STATUS_COLUMN, columns);

    int i = 0;
    settings()->beginWriteArray(KEY_CUSTOM_MOVIE_SCRAPER);
    QMapIterator<MovieScraperInfo, QString> it(m_customMovieScraper);
    while (it.hasNext()) {
        it.next();
        settings()->setArrayIndex(i++);
        settings()->setValue("Info", static_cast<int>(it.key()));
        settings()->setValue("Scraper", it.value());
    }
    settings()->endArray();

    {
        i = 0;
        settings()->beginWriteArray(KEY_CUSTOM_TV_SCRAPER_SHOW);
        QMapIterator<ShowScraperInfo, QString> itTvShow(m_customTvScraperShow);
        while (itTvShow.hasNext()) {
            itTvShow.next();
            settings()->setArrayIndex(i++);
            settings()->setValue("Info", static_cast<int>(itTvShow.key()));
            settings()->setValue("Scraper", itTvShow.value());
        }
        settings()->endArray();
    }
    {
        i = 0;
        settings()->beginWriteArray(KEY_CUSTOM_TV_SCRAPER_EPISODE);
        QMapIterator<EpisodeScraperInfo, QString> itTvEpisode(m_customTvScraperEpisode);
        while (itTvEpisode.hasNext()) {
            itTvEpisode.next();
            settings()->setArrayIndex(i++);
            settings()->setValue("Info", static_cast<int>(itTvEpisode.key()));
            settings()->setValue("Scraper", itTvEpisode.value());
        }
        settings()->endArray();
    }

    settings()->sync();

    emit sigSettingsSaved();
}

/*** GETTER ***/

QSize Settings::importDialogSize()
{
    return m_importDialogSize;
}

QPoint Settings::importDialogPosition()
{
    return m_importDialogPosition;
}

QSize Settings::makeMkvDialogSize()
{
    return m_makeMkvDialogSize;
}

QPoint Settings::makeMkvDialogPosition()
{
    return m_makeMkvDialogPosition;
}

QByteArray Settings::movieDuplicatesSplitterState()
{
    return m_movieDuplicatesSplitterState;
}

DirectorySettings& Settings::directorySettings()
{
    return m_directorySettings;
}


NetworkSettings& Settings::networkSettings()
{
    return m_networkSettings;
}

QStringList Settings::excludeWords()
{
    return value(KEY_EXCLUDE_WORDS).toString().remove(" ").split(",", ElchSplitBehavior::SkipEmptyParts);
}


bool Settings::useYoutubePluginUrls() const
{
    return value(KEY_USE_YOUTUBE_PLUGIN_URL).toBool();
}

bool Settings::autoLoadStreamDetails() const
{
    return value(KEY_AUTO_LOAD_STREAM_DETAILS).toBool();
}

QVector<DataFile> Settings::dataFiles(DataFileType dataType)
{
    QVector<DataFile> files;
    for (const DataFile& file : asConst(m_dataFiles)) {
        if (file.type() == dataType) {
            files.append(file);
        }
    }
    std::sort(files.begin(), files.end(), DataFile::lessThan);
    return files;
}

QVector<DataFile> Settings::dataFiles(ImageType dataType)
{
    return dataFiles(DataFile::dataFileTypeForImageType(dataType));
}

QVector<DataFile> Settings::dataFilesFrodo(DataFileType type)
{
    if (type == DataFileType::NoType) {
        return m_initialDataFilesFrodo;
    }

    QVector<DataFile> files;
    for (const DataFile& file : asConst(m_initialDataFilesFrodo)) {
        if (file.type() == type) {
            files.append(file);
        }
    }
    std::sort(files.begin(), files.end(), DataFile::lessThan);
    return files;
}

bool Settings::usePlotForOutline() const
{
    return value(KEY_USE_PLOT_FOR_OUTLINE).toBool();
}

bool Settings::ignoreDuplicateOriginalTitle() const
{
    return value(KEY_MOVIE_IGNORE_DUPLICATE_ORIGINAL_TITLE).toBool();
}

/*** SETTER ***/

void Settings::setImportDialogSize(QSize size)
{
    m_importDialogSize = size;
    settings()->setValue(KEY_DOWNLOADS_IMPORT_DIALOG_SIZE, size);
}

void Settings::setImportDialogPosition(QPoint position)
{
    m_importDialogPosition = position;
    settings()->setValue(KEY_DOWNLOADS_IMPORT_DIALOG_POSITION, position);
}

void Settings::setMakeMkvDialogSize(QSize size)
{
    m_makeMkvDialogSize = size;
    settings()->setValue(KEY_DOWNLOADS_MAKE_MKV_DIALOG_SIZE, size);
}

void Settings::setMakeMkvDialogPosition(QPoint position)
{
    m_makeMkvDialogPosition = position;
    settings()->setValue(KEY_DOWNLOADS_MAKE_MKV_DIALOG_POSITION, position);
}


void Settings::setMovieDuplicatesSplitterState(QByteArray state)
{
    m_movieDuplicatesSplitterState = state;
    settings()->setValue(KEY_MOVIE_DUPLICATES_SPLITTER_STATE, state);
}

void Settings::setExcludeWords(QString words)
{
    setValue(KEY_EXCLUDE_WORDS, words.remove(" "));
}

void Settings::setUseYoutubePluginUrls(bool use)
{
    setValue(KEY_USE_YOUTUBE_PLUGIN_URL, use);
}

/**
 * \brief Settings::setDataFiles
 */
void Settings::setDataFiles(QVector<DataFile> files)
{
    m_dataFiles = files;
}

void Settings::setAutoLoadStreamDetails(bool autoLoad)
{
    setValue(KEY_AUTO_LOAD_STREAM_DETAILS, autoLoad);
}

void Settings::setUsePlotForOutline(bool use)
{
    setValue(KEY_USE_PLOT_FOR_OUTLINE, use);
}

void Settings::setIgnoreDuplicateOriginalTitle(bool ignoreDuplicateOriginalTitle)
{
    setValue(KEY_MOVIE_IGNORE_DUPLICATE_ORIGINAL_TITLE, ignoreDuplicateOriginalTitle);
}

QSet<ConcertScraperInfo> Settings::scraperInfosConcert(const QString& scraperId)
{
    QSet<ConcertScraperInfo> infos;
    for (const auto& info :
        settings()->value(QString(KEY_SCRAPER_CONCERT_DETAILS).arg(scraperId)).toString().split(",")) {
        infos << ConcertScraperInfo(info.toInt());
    }
    if (!infos.isEmpty() && infos.contains(ConcertScraperInfo::Invalid)) {
        infos.clear();
    }
    return infos;
}

template<>
QSet<MovieScraperInfo> Settings::scraperInfos(QString scraperId)
{
    QSet<MovieScraperInfo> infos;
    for (const auto& info : settings()->value(QString("Scrapers/Movies/%1").arg(scraperId)).toString().split(",")) {
        infos << MovieScraperInfo(info.toInt());
    }
    if (!infos.isEmpty() && infos.contains(MovieScraperInfo::Invalid)) {
        infos.clear();
    }
    return infos;
}

QSet<ShowScraperInfo> Settings::scraperInfosShow(const QString& scraperId)
{
    QSet<ShowScraperInfo> infos;
    for (const auto& info :
        settings()->value(QString(KEY_SCRAPER_TV_SHOW_DETAILS).arg(scraperId)).toString().split(",")) {
        bool ok = false;
        const int val = info.toInt(&ok);
        if (ok) {
            infos << ShowScraperInfo(val);
        }
    }
    // Return ALL available details if non were found.
    // Reason: Users very likely DID NOT un-select all details and then scrape a show.
    // Also if there aren't any details stored yet, the user has to select all details.
    if (infos.isEmpty() || infos.contains(ShowScraperInfo::Invalid)) {
        return mediaelch::allShowScraperInfos();
    }
    return infos;
}

QSet<EpisodeScraperInfo> Settings::scraperInfosEpisode(const QString& scraperId)
{
    QSet<EpisodeScraperInfo> infos;
    for (const auto& info :
        settings()->value(QString(KEY_SCRAPER_TV_EPISODE_DETAILS).arg(scraperId)).toString().split(",")) {
        bool ok = false;
        const int val = info.toInt(&ok);
        if (ok) {
            infos << EpisodeScraperInfo(val);
        }
    }
    // Return ALL available details if non were found.
    // Reason: Users very likely DID NOT un-select all details and then scrape an episode.
    // Also if there aren't any details stored yet, the user has to select all details.
    // And the episode-tab may be "hidden" (the user has to select it).
    if (infos.isEmpty() || infos.contains(EpisodeScraperInfo::Invalid)) {
        return mediaelch::allEpisodeScraperInfos();
    }
    return infos;
}

template<>
QSet<MusicScraperInfo> Settings::scraperInfos(QString scraperId)
{
    QSet<MusicScraperInfo> infos;
    for (const auto& info : settings()->value(QString("Scrapers/Music/%1").arg(scraperId)).toString().split(",")) {
        infos << MusicScraperInfo(info.toInt());
    }
    if (!infos.isEmpty() && infos.contains(MusicScraperInfo::Invalid)) {
        infos.clear();
    }
    return infos;
}

void Settings::setScraperInfos(const QString& scraperNo, const QSet<MovieScraperInfo>& items)
{
    // TODO: Currently based on the Index of the UpdateType combobox.
    //       Better: Pass UpdateType + ScraperID
    QStringList infos;
    infos.reserve(items.size());
    for (const auto info : items) {
        infos << QString::number(static_cast<int>(info));
    }
    settings()->setValue(QString("Scrapers/Movies/%1").arg(scraperNo), infos.join(","));
}

void Settings::setScraperInfosShow(const QString& scraperId, const QSet<ShowScraperInfo>& items)
{
    QStringList infos;
    infos.reserve(items.size());
    for (const auto info : items) {
        infos << QString::number(static_cast<int>(info));
    }
    settings()->setValue(QString(KEY_SCRAPER_TV_SHOW_DETAILS).arg(scraperId), infos.join(","));
}

void Settings::setScraperInfosEpisode(const QString& scraperId, const QSet<EpisodeScraperInfo>& items)
{
    QStringList infos;
    infos.reserve(items.size());
    for (const auto info : items) {
        infos << QString::number(static_cast<int>(info));
    }
    settings()->setValue(QString(KEY_SCRAPER_TV_EPISODE_DETAILS).arg(scraperId), infos.join(","));
}

void Settings::setScraperInfosConcert(const QString& scraperId, const QSet<ConcertScraperInfo>& items)
{
    QStringList infos;
    infos.reserve(items.size());
    for (const auto info : items) {
        infos << QString::number(static_cast<int>(info));
    }
    settings()->setValue(QString(KEY_SCRAPER_CONCERT_DETAILS).arg(scraperId), infos.join(","));
}

void Settings::setScraperInfos(const QString& scraperNo, const QSet<MusicScraperInfo>& items)
{
    QStringList infos;
    infos.reserve(items.size());
    for (const auto info : items) {
        infos << QString::number(static_cast<int>(info));
    }
    settings()->setValue(QString("Scrapers/Music/%1").arg(scraperNo), infos.join(","));
}

bool Settings::downloadActorImages() const
{
    return value(KEY_DOWNLOAD_ACTOR_IMAGES).toBool();
}

void Settings::setDownloadActorImages(bool download)
{
    setValue(KEY_DOWNLOAD_ACTOR_IMAGES, download);
}

void Settings::renamePatterns(RenameType renameType,
    QString& fileNamePattern,
    QString& fileNamePatternMulti,
    QString& directoryPattern,
    QString& seasonPattern,
    QString& replacementDelimiter)
{
    const QString renameTypeStr = renamerTypeToString(renameType);
    QString fileNamePatternDefault = "<title>.<extension>";
    QString fileNamePatternMultiDefault = "<title>-part<partNo>.<extension>";
    if (renameType == RenameType::TvShows) {
        fileNamePatternDefault = "S<season>E<episode> - <title>.<extension>";
        fileNamePatternMultiDefault = "S<season>E<episode> - <title>-part<partNo>.<extension>";
    }
    fileNamePattern =
        settings()->value(QString("RenamePattern/%1/FileName").arg(renameTypeStr), fileNamePatternDefault).toString();
    fileNamePatternMulti =
        settings()
            ->value(QString("RenamePattern/%1/FileNameMulti").arg(renameTypeStr), fileNamePatternMultiDefault)
            .toString();
    directoryPattern = settings()
                           ->value(QString("RenamePattern/%1/DirectoryPattern").arg(renameTypeStr), "<title> (<year>)")
                           .toString();
    seasonPattern =
        settings()->value(QString("RenamePattern/%1/SeasonPattern").arg(renameTypeStr), "Season <season>").toString();

    replacementDelimiter = settings()->value(QString("RenamePattern/%1/Delimiter").arg(renameTypeStr), "_").toString();
}

void Settings::setRenamePatterns(RenameType renameType,
    QString fileNamePattern,
    QString fileNamePatternMulti,
    QString directoryPattern,
    QString seasonPattern,
    QString replacementDelimiterPattern)
{
    const QString renameTypeStr = renamerTypeToString(renameType);
    settings()->setValue(QString("RenamePattern/%1/FileName").arg(renameTypeStr), fileNamePattern);
    settings()->setValue(QString("RenamePattern/%1/FileNameMulti").arg(renameTypeStr), fileNamePatternMulti);
    settings()->setValue(QString("RenamePattern/%1/DirectoryPattern").arg(renameTypeStr), directoryPattern);
    settings()->setValue(QString("RenamePattern/%1/SeasonPattern").arg(renameTypeStr), seasonPattern);
    settings()->setValue(QString("RenamePattern/%1/Delimiter").arg(renameTypeStr), replacementDelimiterPattern);
}

void Settings::setRenamings(RenameType renameType,
    bool files,
    bool folders,
    bool seasonDirectories,
    bool replaceDelimiter)
{
    const QString renameTypeStr = renamerTypeToString(renameType);
    settings()->setValue(QString("RenamePattern/%1/RenameFiles").arg(renameTypeStr), files);
    settings()->setValue(QString("RenamePattern/%1/RenameFolders").arg(renameTypeStr), folders);
    settings()->setValue(QString("RenamePattern/%1/UseSeasonDirectories").arg(renameTypeStr), seasonDirectories);
    settings()->setValue(QString("RenamePattern/%1/ReplaceDelimiter").arg(renameTypeStr), replaceDelimiter);
}

void Settings::renamings(RenameType renameType,
    bool& files,
    bool& folders,
    bool& seasonDirectories,
    bool& replaceDelimiter)
{
    const QString renameTypeStr = renamerTypeToString(renameType);
    files = settings()->value(QString("RenamePattern/%1/RenameFiles").arg(renameTypeStr), true).toBool();
    folders = settings()->value(QString("RenamePattern/%1/RenameFolders").arg(renameTypeStr), true).toBool();
    seasonDirectories =
        settings()->value(QString("RenamePattern/%1/UseSeasonDirectories").arg(renameTypeStr), true).toBool();
    replaceDelimiter =
        settings()->value(QString("RenamePattern/%1/ReplaceDelimiter").arg(renameTypeStr), false).toBool();
}

int Settings::tvShowUpdateOption()
{
    return value(KEY_TV_SHOW_UPDATE_OPTION).toInt();
}

void Settings::setTvShowUpdateOption(int option)
{
    setValue(KEY_TV_SHOW_UPDATE_OPTION, option);
}

AdvancedSettings* Settings::advanced()
{
    return &m_advancedSettings;
}

bool Settings::ignoreArticlesWhenSorting() const
{
    return value(KEY_IGNORE_ARTICLES_WHEN_SORTING).toBool();
}

void Settings::setIgnoreArticlesWhenSorting(bool ignore)
{
    setValue(KEY_IGNORE_ARTICLES_WHEN_SORTING, ignore);
}

void Settings::setMovieSetArtworkType(MovieSetArtworkType type)
{
    setValue(KEY_MOVIE_SET_ARTWORK_STORING_TYPE, static_cast<int>(type));
}

MovieSetArtworkType Settings::movieSetArtworkType() const
{
    return MovieSetArtworkType(value(KEY_MOVIE_SET_ARTWORK_STORING_TYPE).toInt());
}

void Settings::setMovieSetArtworkDirectory(mediaelch::DirectoryPath dir)
{
    setValue(KEY_MOVIE_SET_ARTWORK_DIRECTORY, dir.toString());
}

mediaelch::DirectoryPath Settings::movieSetArtworkDirectory() const
{
    return mediaelch::DirectoryPath(value(KEY_MOVIE_SET_ARTWORK_DIRECTORY).toString());
}

void Settings::setMediaStatusColumn(QVector<MediaStatusColumn> columns)
{
    m_mediaStatusColumns = columns;
}

QVector<MediaStatusColumn> Settings::mediaStatusColumns() const
{
    return m_mediaStatusColumns;
}

SeasonOrder Settings::seasonOrder() const
{
    return SeasonOrder(value(KEY_TV_SHOWS_SEASON_ORDER).toInt());
}

void Settings::setSeasonOrder(SeasonOrder order)
{
    setValue(KEY_TV_SHOWS_SEASON_ORDER, static_cast<int>(order));
    saveSettings();
}

void Settings::setDontShowDeleteImageConfirm(bool show)
{
    setValue(KEY_WARNINGS_DO_NOT_SHOW_DELETE_IMAGE_CONFIRM, show);
    saveSettings();
}

bool Settings::dontShowDeleteImageConfirm() const
{
    return value(KEY_WARNINGS_DO_NOT_SHOW_DELETE_IMAGE_CONFIRM).toBool();
}

const QMap<MovieScraperInfo, QString>& Settings::customMovieScraper() const
{
    return m_customMovieScraper;
}

void Settings::setCustomMovieScraper(QMap<MovieScraperInfo, QString> customMovieScraper)
{
    m_customMovieScraper = customMovieScraper;
}

const QMap<ShowScraperInfo, QString>& Settings::customTvScraperShow() const
{
    return m_customTvScraperShow;
}

void Settings::setCustomTvScraperShow(QMap<ShowScraperInfo, QString> customTvScraper)
{
    // Ensure that all details are set. Default is TmdbTv because it supports most (was previously TheTvDb).
    for (const ShowScraperInfo info : mediaelch::allShowScraperInfos()) {
        if (!customTvScraper.contains(info)) {
            customTvScraper.insert(info, mediaelch::scraper::TmdbTv::ID);
        }
    }
    m_customTvScraperShow = customTvScraper;
}

const QMap<EpisodeScraperInfo, QString>& Settings::customTvScraperEpisode() const
{
    return m_customTvScraperEpisode;
}

void Settings::setCustomTvScraperEpisode(QMap<EpisodeScraperInfo, QString> customTvScraper)
{
    // Ensure that all details are set. Default is TmdbTv because it supports most (was previously TheTvDb).
    for (const EpisodeScraperInfo info : mediaelch::allEpisodeScraperInfos()) {
        if (!customTvScraper.contains(info)) {
            customTvScraper.insert(info, mediaelch::scraper::TmdbTv::ID);
        }
    }
    m_customTvScraperEpisode = customTvScraper;
}

int Settings::currentMovieScraper() const
{
    return value(KEY_SCRAPER_CURRENT_MOVIE_SCRAPER).toInt();
}

void Settings::setCurrentMovieScraper(int current)
{
    setValue(KEY_SCRAPER_CURRENT_MOVIE_SCRAPER, current);
    settings()->sync();
}

QString Settings::currentTvShowScraper() const
{
    return value(KEY_SCRAPER_CURRENT_TV_SHOW_SCRAPER).toString();
}

QString Settings::currentConcertScraper() const
{
    return value(KEY_SCRAPER_CURRENT_CONCERT_SCRAPER).toString();
}

void Settings::setCurrentTvShowScraper(const QString& current)
{
    setValue(KEY_SCRAPER_CURRENT_TV_SHOW_SCRAPER, current);
    settings()->sync();
}

void Settings::setCurrentConcertScraper(const QString& current)
{
    setValue(KEY_SCRAPER_CURRENT_CONCERT_SCRAPER, current);
    settings()->sync();
}

void Settings::setKeepDownloadSource(bool keep)
{
    setValue(KEY_DOWNLOADS_KEEP_SOURCE, keep);
}

bool Settings::keepDownloadSource() const
{
    return value(KEY_DOWNLOADS_KEEP_SOURCE).toBool();
}

void Settings::setCheckForUpdates(bool check)
{
    setValue(KEY_CHECK_FOR_UPDATES, check);
}

bool Settings::checkForUpdates() const
{
    return value(KEY_CHECK_FOR_UPDATES).toBool();
}

void Settings::setShowMissingEpisodesHint(bool show)
{
    setValue(KEY_TV_SHOWS_SHOW_MISSING_EPISODES, show);
}

bool Settings::showMissingEpisodesHint() const
{
    return value(KEY_TV_SHOWS_SHOW_MISSING_EPISODES).toBool();
}

void Settings::setMultiScrapeOnlyWithId(bool onlyWithId)
{
    setValue(KEY_MOVIES_MULTI_SCRAPE_ONLY_WITH_ID, onlyWithId);
}

bool Settings::multiScrapeOnlyWithId() const
{
    return value(KEY_MOVIES_MULTI_SCRAPE_ONLY_WITH_ID).toBool();
}

void Settings::setMultiScrapeSaveEach(bool saveEach)
{
    setValue(KEY_MOVIES_MULTI_SCRAPE_SAVE_EACH, saveEach);
}

bool Settings::multiScrapeSaveEach() const
{
    return value(KEY_MOVIES_MULTI_SCRAPE_SAVE_EACH).toBool();
}

QString Settings::applicationDir()
{
    return QApplication::applicationDirPath();
}

mediaelch::DirectoryPath Settings::databaseDir()
{
    if (advanced()->portableMode()) {
        return mediaelch::DirectoryPath(applicationDir());
    }
    return mediaelch::DirectoryPath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
}

mediaelch::DirectoryPath Settings::imageCacheDir()
{
    if (advanced()->portableMode()) {
        return mediaelch::DirectoryPath(applicationDir());
    }
    return mediaelch::DirectoryPath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
}

mediaelch::DirectoryPath Settings::exportTemplatesDir()
{
    if (advanced()->portableMode()) {
        return mediaelch::DirectoryPath(applicationDir() + QDir::separator() + "export_themes");
    }
    return mediaelch::DirectoryPath(
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QDir::separator() + "export_themes");
}

void Settings::setShowAdultScrapers(bool show)
{
    setValue(KEY_SCRAPERS_SHOW_ADULT, show);
}

bool Settings::showAdultScrapers() const
{
    return value(KEY_SCRAPERS_SHOW_ADULT).toBool();
}

void Settings::setStartupSection(QString startupSection)
{
    setValue(KEY_STARTUP_SECTION, startupSection);
}

QString Settings::startupSection()
{
    return value(KEY_STARTUP_SECTION).toString();
}

void Settings::setDonated(bool donated)
{
    setValue(KEY_DONATED, donated);
    settings()->sync();
    emit sigDonated(donated);
}

bool Settings::donated() const
{
    return value(KEY_DONATED).toBool();
}

void Settings::setTheme(QString theme)
{
    setValue(KEY_THEME, theme);
}

QString Settings::theme()
{
    return value(KEY_THEME).toString();
}

void Settings::setLastImagePath(mediaelch::DirectoryPath path)
{
    // Also save in this setter, because this method is called in ImageDialog as well.
    setValue(KEY_LAST_IMAGE_PATH, path.toString());
    settings()->sync();
}

mediaelch::DirectoryPath Settings::lastImagePath()
{
    return mediaelch::DirectoryPath(value(KEY_LAST_IMAGE_PATH).toString());
}

int Settings::extraFanartsMusicArtists() const
{
    return value(KEY_MUSIC_ARTISTS_EXTRA_FANARTS).toInt();
}

void Settings::setExtraFanartsMusicArtists(int extraFanartsMusicArtists)
{
    setValue(KEY_MUSIC_ARTISTS_EXTRA_FANARTS, extraFanartsMusicArtists);
}

void Settings::onSettingChanged(Settings::Key key, QObject* context, std::function<void()> callback)
{
    MediaElch_Expects(context != nullptr);

    if (!m_callbacks.contains(key)) {
        m_callbacks.insert(key, {});
    }
    m_callbacks[key].push_back({context, std::move(callback)});

    // To avoid use-after-free, delete the callbacks if the context was destroyed.
    connect(context, &QObject::destroyed, this, [key, context, this]() {
        if (m_callbacks.contains(key)) {
            auto& c = m_callbacks[key];
            auto contextsMatch = [context](const auto& x) { //
                return x.first == context;
            };
            c.erase(std::remove_if(c.begin(), c.end(), contextsMatch), c.end());
        }
    });
}

void Settings::beginTransaction()
{
    if (m_isTransactionInProgress) {
        qCCritical(generic) << "[Settings] beginTransaction(): already in progress!";
        MediaElch_Debug_Expects(!m_isTransactionInProgress);
        return;
    }

    m_localItems = m_items;
    m_isTransactionInProgress = true;
}

void Settings::commitTransaction()
{
    if (!m_isTransactionInProgress) {
        qCCritical(generic) << "[Settings] commitTransaction(): was not in progress!";
        MediaElch_Debug_Expects(m_isTransactionInProgress);
        return;
    }

    m_isTransactionInProgress = false;

    for (auto& localItem : m_localItems) {
        Item& item = findItem(localItem.key);
        if (item.value == localItem.value) {
            // nothing has changed -> no need to write anything
            continue;
        }

        if (item.isNull()) {
            addItem(localItem);
        } else {
            item.value = localItem.value;
        }

        writeValueToDisk(localItem.key, localItem.value);
    }

    m_localItems.clear();
}

void Settings::abortTransaction()
{
    if (!m_isTransactionInProgress) {
        qCWarning(generic) << "[Settings] abortTransaction(): was not in progress!";
        MediaElch_Debug_Expects(m_isTransactionInProgress);
        return;
    }
    m_isTransactionInProgress = false;

    for (auto& localItem : m_localItems) {
        Item item = findItem(localItem.key);
        if (!item.isNull() && item.value == localItem.value) {
            continue;
        }
        // reset means we need to notify subscribers
        emitChangeFor(localItem.key);
    }

    m_localItems.clear();
}

void Settings::emitChangeFor(const Settings::Key& key) const
{
    auto iter = m_callbacks.find(key);
    if (iter != m_callbacks.end()) {
        auto& callbacks = iter.value();
        for (auto& callback : callbacks) {
            callback.second();
        }
    }
}

Settings::Item& Settings::findItem(const Settings::Key& key)
{
    Items& items = m_isTransactionInProgress ? m_localItems : m_items;

    auto it = items.find(key);
    if (it == items.end()) {
        static Item nullItem;
        return nullItem;
    }

    return it.value();
}

const Settings::Item& Settings::findItem(const Settings::Key& key) const
{
    const Items& items = m_isTransactionInProgress ? m_localItems : m_items;

    auto it = items.find(key);
    if (it == items.end()) {
        static const Item nullItem;
        return nullItem;
    }

    return it.value();
}

void Settings::writeValueToDisk(const Settings::Key& key, const Settings::Value& value)
{
    settings()->setValue(key.key, value);
}

void Settings::readAllItemsFromDisk()
{
    for (const QString& settingsKey : m_settings->allKeys()) {
        Key key = Key({}, settingsKey);
        Value value = settings()->value(settingsKey);

        Item& item = findItem(key);
        if (item.isNull()) {
            Item newItem;
            newItem.key = key;
            newItem.value = std::move(value);

            m_items[key] = newItem;
        } else {
            item.value = value;
        }
    }
}

void Settings::addItem(Settings::Item item)
{
    MediaElch_Debug_Expects(!item.isNull());
    if (m_isTransactionInProgress) {
        m_localItems[item.key] = item;
    } else {
        m_items[item.key] = item;
    }
}

const Settings::Items& Settings::items() const
{
    return m_isTransactionInProgress ? m_localItems : m_items;
}
