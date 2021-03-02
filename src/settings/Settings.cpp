#include "Settings.h"

#include "globals/Manager.h"
#include "globals/ScraperInfos.h"
#include "renamer/RenamerDialog.h"
#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/music/MusicScraper.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "settings/AdvancedSettingsXmlReader.h"

#include <QApplication>
#include <QDesktopServices>
#include <QMutex>
#include <QMutexLocker>

static constexpr char KEY_ALL_DATA_FILES[] = "AllDataFiles";
static constexpr char KEY_AUTO_LOAD_STREAM_DETAILS[] = "AutoLoadStreamDetails";
static constexpr char KEY_CHECK_FOR_UPDATES[] = "CheckForUpdates";
static constexpr char KEY_CUSTOM_MOVIE_SCRAPER[] = "CustomMovieScraper";
static constexpr char KEY_CUSTOM_TV_SCRAPER_SHOW[] = "CustomTvScraperShow";
static constexpr char KEY_CUSTOM_TV_SCRAPER_EPISODE[] = "CustomTvScraperEpisode";
static constexpr char KEY_DEBUG_MODE_ACTIVATED[] = "DebugModeActivated";
static constexpr char KEY_DONATED[] = "Donated";

static constexpr char KEY_CSV_EXPORT_SEPARATOR[] = "CsvExport/Separator";
static constexpr char KEY_CSV_EXPORT_REPLACEMENT[] = "CsvExport/Replacement";
static constexpr char KEY_CSV_EXPORT_TYPES[] = "CsvExport/Types";
static constexpr char KEY_CSV_EXPORT_MOVIE_FIELDS[] = "CsvExport/MovieFields";
static constexpr char KEY_CSV_EXPORT_TV_SHOW_FIELDS[] = "CsvExport/TvShowFields";
static constexpr char KEY_CSV_EXPORT_TV_EPISODE_FIELDS[] = "CsvExport/TvEpisodeFields";
static constexpr char KEY_CSV_EXPORT_CONCERT_FIELDS[] = "CsvExport/ConcertFields";
static constexpr char KEY_CSV_EXPORT_MUSIC_ARTIST_FIELDS[] = "CsvExport/MusicArtistFields";
static constexpr char KEY_CSV_EXPORT_MUSIC_ALBUM_FIELDS[] = "CsvExport/MusicAlbumFields";

static constexpr char KEY_DOWNLOAD_ACTOR_IMAGES[] = "DownloadActorImages";
static constexpr char KEY_DOWNLOADS_DELETE_ARCHIVES[] = "Downloads/DeleteArchives";
static constexpr char KEY_DOWNLOADS_IMPORT_DIALOG_POSITION[] = "Downloads/ImportDialogPosition";
static constexpr char KEY_DOWNLOADS_IMPORT_DIALOG_SIZE[] = "Downloads/ImportDialogSize";
static constexpr char KEY_DOWNLOADS_KEEP_SOURCE[] = "Downloads/KeepSource";
static constexpr char KEY_DOWNLOADS_MAKE_MKV_DIALOG_POSITION[] = "Downloads/MakeMkvDialogPosition";
static constexpr char KEY_DOWNLOADS_MAKE_MKV_DIALOG_SIZE[] = "Downloads/MakeMkvDialogSize";
static constexpr char KEY_EXCLUDE_WORDS[] = "excludeWords";
static constexpr char KEY_IGNORE_ARTICLES_WHEN_SORTING[] = "IgnoreArticlesWhenSorting";
static constexpr char KEY_IS_MAIN_WINDOW_MAXIMIZED[] = "MainWindowMaximized";
static constexpr char KEY_LAST_IMAGE_PATH[] = "LastImagePath";
static constexpr char KEY_MAIN_SPLITTER_STATE[] = "MainSplitterState";
static constexpr char KEY_MAIN_WINDOW_POSITION[] = "MainWindowPosition";
static constexpr char KEY_MAIN_WINDOW_SIZE[] = "MainWindowSize";
static constexpr char KEY_MEDIA_STATUS_COLUMN[] = "MediaStatusColumn";
static constexpr char KEY_MOVIE_DUPLICATES_SPLITTER_STATE[] = "MovieDuplicatesSplitterState";
static constexpr char KEY_MOVIE_IGNORE_DUPLICATE_ORIGINAL_TITLE[] = "Movies/IgnoreDuplicateOriginalTitle";
static constexpr char KEY_MOVIE_SET_ARTWORK_DIRECTORY[] = "MovieSetArtwork/Directory";
static constexpr char KEY_MOVIE_SET_ARTWORK_STORING_TYPE[] = "MovieSetArtwork/StoringType";
static constexpr char KEY_MOVIES_MULTI_SCRAPE_ONLY_WITH_ID[] = "Movies/MultiScrapeOnlyWithId";
static constexpr char KEY_MOVIES_MULTI_SCRAPE_SAVE_EACH[] = "Movies/MultiScrapeSaveEach";
static constexpr char KEY_MUSIC_ARTISTS_EXTRA_FANARTS[] = "Music/Artists/ExtraFanarts";
static constexpr char KEY_SCRAPER_CURRENT_MOVIE_SCRAPER[] = "Scraper/CurrentMovieScraper";
static constexpr char KEY_SCRAPER_CURRENT_TV_SHOW_SCRAPER[] = "Scraper/CurrentTvShowScraper";
static constexpr char KEY_SCRAPER_CURRENT_CONCERT_SCRAPER[] = "Scraper/CurrentConcertScraper";
static constexpr char KEY_SCRAPER_TV_SHOW_DETAILS[] = "Scrapers/TvShows/%1";
static constexpr char KEY_SCRAPER_TV_EPISODE_DETAILS[] = "Scrapers/Episodes/%1";
static constexpr char KEY_SCRAPER_CONCERT_DETAILS[] = "Scrapers/Concerts/%1";
static constexpr char KEY_SCRAPERS_SHOW_ADULT[] = "Scrapers/ShowAdult";
static constexpr char KEY_SETTINGS_WINDOW_POSITION[] = "SettingsWindowPosition";
static constexpr char KEY_SETTINGS_WINDOW_SIZE[] = "SettingsWindowSize";
static constexpr char KEY_STARTUP_SECTION[] = "StartupSection";
static constexpr char KEY_TV_SHOW_UPDATE_OPTION[] = "TvShowUpdateOption";
static constexpr char KEY_TV_SHOWS_SEASON_ORDER[] = "TvShows/SeasonOrder";
static constexpr char KEY_TV_SHOWS_SHOW_MISSING_EPISODES[] = "TvShows/ShowMissingEpisodesHint";
static constexpr char KEY_USE_PLOT_FOR_OUTLINE[] = "Movies/UsePlotForOutline";
static constexpr char KEY_USE_YOUTUBE_PLUGIN_URL[] = "UseYoutubePluginURLs";
static constexpr char KEY_WARNINGS_DO_NOT_SHOW_DELETE_IMAGE_CONFIRM[] = "Warnings/DontShowDeleteImageConfirm";


Settings::Settings(QObject* parent) : QObject(parent)
{
    auto advancedSettingsPair = AdvancedSettingsXmlReader::loadFromDefaultPath();
    m_advancedSettings = std::move(advancedSettingsPair.first);

    qDebug() << m_advancedSettings;

    if (m_advancedSettings.portableMode()) {
        qDebug() << "[Windows] Using portable mode!";
        m_settings = new QSettings(Settings::applicationDir() + "/MediaElch.ini", QSettings::IniFormat, this);
    } else {
        m_settings = new QSettings(this);
    }

    m_directorySettings.setQSettings(m_settings);
    m_kodiSettings.setQSettings(m_settings);
    m_importSettings.setQSettings(m_settings);
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
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieSetPoster, "<baseFileName>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieSetBackdrop, "<baseFileName>-fanart.jpg", 0));

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

ScraperSettings* Settings::scraperSettings(const QString& id)
{
    std::string idStd = id.toStdString();
    if (m_scraperSettings.find(idStd) == m_scraperSettings.cend()) {
        qCritical() << "[TvScraperSettingsWidget] Missing settings entry in settings map!";
        return nullptr;
    }
    return m_scraperSettings[idStd].get();
}

/**
 * \brief Loads all settings
 */
void Settings::loadSettings()
{
    // Globals
    m_mainWindowSize = settings()->value(KEY_MAIN_WINDOW_SIZE).toSize();
    m_settingsWindowSize = settings()->value(KEY_SETTINGS_WINDOW_SIZE).toSize();
    m_mainWindowMaximized = settings()->value(KEY_IS_MAIN_WINDOW_MAXIMIZED).toBool();
    m_mainSplitterState = settings()->value(KEY_MAIN_SPLITTER_STATE).toByteArray();
    m_movieDuplicatesSplitterState = settings()->value(KEY_MOVIE_DUPLICATES_SPLITTER_STATE).toByteArray();
    m_debugModeActivated = settings()->value(KEY_DEBUG_MODE_ACTIVATED, false).toBool();
    m_autoLoadStreamDetails = settings()->value(KEY_AUTO_LOAD_STREAM_DETAILS, true).toBool();
    m_usePlotForOutline = settings()->value(KEY_USE_PLOT_FOR_OUTLINE, true).toBool();
    m_ignoreDuplicateOriginalTitle = settings()->value(KEY_MOVIE_IGNORE_DUPLICATE_ORIGINAL_TITLE, true).toBool();
    m_downloadActorImages = settings()->value(KEY_DOWNLOAD_ACTOR_IMAGES, true).toBool();
    m_ignoreArticlesWhenSorting = settings()->value(KEY_IGNORE_ARTICLES_WHEN_SORTING, false).toBool();
    m_checkForUpdates = settings()->value(KEY_CHECK_FOR_UPDATES, true).toBool();
    m_showAdultScrapers = settings()->value(KEY_SCRAPERS_SHOW_ADULT, false).toBool();
    m_startupSection = settings()->value(KEY_STARTUP_SECTION, "movies").toString();
    m_donated = settings()->value(KEY_DONATED, false).toBool();
    m_lastImagePath = settings()->value(KEY_LAST_IMAGE_PATH, QDir::homePath()).toString();

    // Window positions
    m_mainWindowPosition = fixWindowPosition(settings()->value(KEY_MAIN_WINDOW_POSITION).toPoint());
    m_settingsWindowPosition = fixWindowPosition(settings()->value(KEY_SETTINGS_WINDOW_POSITION).toPoint());
    m_importDialogPosition = fixWindowPosition(settings()->value(KEY_DOWNLOADS_IMPORT_DIALOG_POSITION).toPoint());
    m_makeMkvDialogPosition = fixWindowPosition(settings()->value(KEY_DOWNLOADS_MAKE_MKV_DIALOG_POSITION).toPoint());

    // Tv Shows
    m_seasonOrder = SeasonOrder(settings()->value(KEY_TV_SHOWS_SEASON_ORDER, 1).toInt());

    // Warnings
    m_dontShowDeleteImageConfirm = settings()->value(KEY_WARNINGS_DO_NOT_SHOW_DELETE_IMAGE_CONFIRM, false).toBool();

    m_directorySettings.loadSettings();
    m_kodiSettings.loadSettings();
    m_importSettings.loadSettings();
    m_networkSettings.loadSettings();

    m_excludeWords =
        settings()->value(KEY_EXCLUDE_WORDS).toString().remove(" ").split(",", ElchSplitBehavior::SkipEmptyParts);

    if (m_excludeWords.isEmpty()) {
        m_excludeWords = QStringLiteral(
            "ac3,dts,custom,dc,divx,divx5,dsr,dsrip,dutch,dvd,dvdrip,dvdscr,dvdscreener,screener,dvdivx,"
            "cam,fragment,fs,hdtv,hdrip,hdtvrip,internal,limited,"
            "multisubs,ntsc,ogg,ogm,pal,pdtv,proper,repack,rerip,retail,r3,r5,bd5,se,svcd,swedish,german,"
            "nfofix,unrated,ws,telesync,ts,telecine,tc,"
            "brrip,bdrip,480p,480i,576p,576i,720p,720i,1080p,1080i,hrhd,hrhdtv,hddvd,bluray,x264,h264,"
            "xvid,xvidvd,xxx,www,mkv")
                             .split(",", ElchSplitBehavior::SkipEmptyParts);
    }

    const auto loadSettings = [&](auto scrapers) {
        for (auto* scraper : scrapers) {
            if (scraper->hasSettings()) {
                std::string id = scraper->identifier().toStdString();
                // may replace existing settings
                m_scraperSettings[id] = std::make_unique<ScraperSettingsQt>(scraper->identifier(), *m_settings);
                scraper->loadSettings(*m_scraperSettings[id]);
            }
        }
    };
    loadSettings(Manager::instance()->scrapers().musicScrapers());

    // new version
    const auto loadSettings2 = [&](auto scrapers) {
        for (auto* scraper : scrapers) {
            if (scraper->hasSettings()) {
                std::string id = scraper->meta().identifier.toStdString();
                // may replace existing settings
                m_scraperSettings[id] = std::make_unique<ScraperSettingsQt>(scraper->meta().identifier, *m_settings);
                scraper->loadSettings(*m_scraperSettings[id]);
            }
        }
    };
    loadSettings2(Manager::instance()->scrapers().movieScrapers());
    loadSettings2(Manager::instance()->scrapers().concertScrapers());
    loadSettings2(Manager::instance()->imageProviders());

    // TV scraper settings
    for (auto* scraper : Manager::instance()->scrapers().tvScrapers()) {
        const QString id = scraper->meta().identifier;
        m_scraperSettings[id.toStdString()] = std::make_unique<ScraperSettingsQt>(id, *m_settings);
        // Not loaded on initial start up but per request.
    }

    m_currentMovieScraper = settings()->value(KEY_SCRAPER_CURRENT_MOVIE_SCRAPER, 0).toInt();
    m_currentTvShowScraper = settings()->value(KEY_SCRAPER_CURRENT_TV_SHOW_SCRAPER, 0).toString();
    m_currentConcertScraper = settings()->value(KEY_SCRAPER_CURRENT_CONCERT_SCRAPER, 0).toString();

    // Media Centers
    m_youtubePluginUrls = settings()->value(KEY_USE_YOUTUBE_PLUGIN_URL, false).toBool();

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
    m_movieSetArtworkType = MovieSetArtworkType(settings()->value(KEY_MOVIE_SET_ARTWORK_STORING_TYPE, 0).toInt());
    m_movieSetArtworkDirectory = settings()->value(KEY_MOVIE_SET_ARTWORK_DIRECTORY).toString();

    // Media Status Columns
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
    // Ensure that all details are set. Default is TheTvDb because it supports most.
    for (const ShowScraperInfo info : mediaelch::allShowScraperInfos()) {
        if (!m_customTvScraperShow.contains(info)) {
            m_customTvScraperShow.insert(info, mediaelch::scraper::TheTvDb::ID);
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
    // Ensure that all details are set. Default is TheTvDb because it supports most.
    const auto allEpisodeInfos = mediaelch::allEpisodeScraperInfos();
    for (const EpisodeScraperInfo info : allEpisodeInfos) {
        if (!m_customTvScraperEpisode.contains(info)) {
            m_customTvScraperEpisode.insert(info, mediaelch::scraper::TheTvDb::ID);
        }
    }
    settings()->endArray();

    // ------------------------------------------------------------------------

    m_csvExportSeparator = settings()->value(KEY_CSV_EXPORT_SEPARATOR).toString();
    m_csvExportReplacement = settings()->value(KEY_CSV_EXPORT_REPLACEMENT).toString();
    m_csvExportTypes = settings()->value(KEY_CSV_EXPORT_TYPES).toString().split(",", ElchSplitBehavior::SkipEmptyParts);
    m_csvExportMovieFields =
        settings()->value(KEY_CSV_EXPORT_MOVIE_FIELDS).toString().split(",", ElchSplitBehavior::SkipEmptyParts);
    m_csvExportTvShowFields =
        settings()->value(KEY_CSV_EXPORT_TV_SHOW_FIELDS).toString().split(",", ElchSplitBehavior::SkipEmptyParts);
    m_csvExportTvEpisodeFields =
        settings()->value(KEY_CSV_EXPORT_TV_EPISODE_FIELDS).toString().split(",", ElchSplitBehavior::SkipEmptyParts);
    m_csvExportConcertFields =
        settings()->value(KEY_CSV_EXPORT_CONCERT_FIELDS).toString().split(",", ElchSplitBehavior::SkipEmptyParts);
    m_csvExportMusicArtistFields =
        settings()->value(KEY_CSV_EXPORT_MUSIC_ARTIST_FIELDS).toString().split(",", ElchSplitBehavior::SkipEmptyParts);
    m_csvExportMusicAlbumFields =
        settings()->value(KEY_CSV_EXPORT_MUSIC_ALBUM_FIELDS).toString().split(",", ElchSplitBehavior::SkipEmptyParts);

    // Downloads
    m_deleteArchives = settings()->value(KEY_DOWNLOADS_DELETE_ARCHIVES, false).toBool();
    m_importDialogSize = settings()->value(KEY_DOWNLOADS_IMPORT_DIALOG_SIZE).toSize();
    m_makeMkvDialogSize = settings()->value(KEY_DOWNLOADS_MAKE_MKV_DIALOG_SIZE).toSize();
    m_keepDownloadSource = settings()->value(KEY_DOWNLOADS_KEEP_SOURCE, true).toBool();

    // Movies
    m_multiScrapeOnlyWithId = settings()->value(KEY_MOVIES_MULTI_SCRAPE_ONLY_WITH_ID, false).toBool();
    m_multiScrapeSaveEach = settings()->value(KEY_MOVIES_MULTI_SCRAPE_SAVE_EACH, false).toBool();

    m_showMissingEpisodesHint = settings()->value(KEY_TV_SHOWS_SHOW_MISSING_EPISODES, true).toBool();

    m_extraFanartsMusicArtists = settings()->value(KEY_MUSIC_ARTISTS_EXTRA_FANARTS, 0).toInt();
}

void Settings::saveSettings()
{
    settings()->setValue(KEY_DEBUG_MODE_ACTIVATED, m_debugModeActivated);
    settings()->setValue(KEY_AUTO_LOAD_STREAM_DETAILS, m_autoLoadStreamDetails);

    settings()->setValue(KEY_USE_YOUTUBE_PLUGIN_URL, m_youtubePluginUrls);
    settings()->setValue(KEY_USE_PLOT_FOR_OUTLINE, m_usePlotForOutline);
    settings()->setValue(KEY_MOVIE_IGNORE_DUPLICATE_ORIGINAL_TITLE, m_ignoreDuplicateOriginalTitle);
    settings()->setValue(KEY_DOWNLOAD_ACTOR_IMAGES, m_downloadActorImages);
    settings()->setValue(KEY_IGNORE_ARTICLES_WHEN_SORTING, m_ignoreArticlesWhenSorting);
    settings()->setValue(KEY_CHECK_FOR_UPDATES, m_checkForUpdates);
    settings()->setValue(KEY_SCRAPERS_SHOW_ADULT, m_showAdultScrapers);
    settings()->setValue(KEY_STARTUP_SECTION, m_startupSection);
    settings()->setValue(KEY_DONATED, m_donated);
    settings()->setValue(KEY_LAST_IMAGE_PATH, m_lastImagePath.toString());


    // Tv Shows
    settings()->setValue(KEY_TV_SHOWS_SEASON_ORDER, static_cast<int>(m_seasonOrder));

    // Warnings
    settings()->setValue(KEY_WARNINGS_DO_NOT_SHOW_DELETE_IMAGE_CONFIRM, m_dontShowDeleteImageConfirm);

    m_directorySettings.saveSettings();
    m_kodiSettings.saveSettings();
    m_importSettings.saveSettings();
    m_networkSettings.saveSettings();

    settings()->setValue(KEY_EXCLUDE_WORDS, m_excludeWords.join(","));

    const auto saveSettings = [&](auto scrapers) {
        for (auto* scraper : scrapers) {
            if (scraper->hasSettings()) {
                std::string id = scraper->identifier().toStdString();
                scraper->saveSettings(*m_scraperSettings[id]);
                m_scraperSettings[id]->save();
            }
        }
    };
    saveSettings(Manager::instance()->scrapers().musicScrapers());

    const auto saveSettings2 = [&](auto scrapers) {
        for (auto* scraper : scrapers) {
            if (scraper->hasSettings()) {
                std::string id = scraper->meta().identifier.toStdString();
                scraper->saveSettings(*m_scraperSettings[id]);
                m_scraperSettings[id]->save();
            }
        }
    };
    saveSettings2(Manager::instance()->scrapers().movieScrapers());
    saveSettings2(Manager::instance()->scrapers().concertScrapers());
    saveSettings2(Manager::instance()->imageProviders());

    // TV scraper settings
    for (auto* scraper : Manager::instance()->scrapers().tvScrapers()) {
        // Settings may have been changed somewhere else.
        m_scraperSettings[scraper->meta().identifier.toStdString()]->save();
    }

    settings()->setValue(KEY_SCRAPER_CURRENT_MOVIE_SCRAPER, m_currentMovieScraper);
    settings()->setValue(KEY_SCRAPER_CURRENT_TV_SHOW_SCRAPER, m_currentTvShowScraper);
    settings()->setValue(KEY_SCRAPER_CURRENT_CONCERT_SCRAPER, m_currentConcertScraper);

    settings()->beginWriteArray(KEY_ALL_DATA_FILES);
    for (int i = 0, n = m_dataFiles.count(); i < n; ++i) {
        settings()->setArrayIndex(i);
        settings()->setValue("type", static_cast<int>(m_dataFiles.at(i).type()));
        settings()->setValue("fileName", m_dataFiles.at(i).fileName());
        settings()->setValue("pos", m_dataFiles.at(i).pos());
    }
    settings()->endArray();

    settings()->setValue(KEY_MOVIE_SET_ARTWORK_STORING_TYPE, static_cast<int>(m_movieSetArtworkType));
    settings()->setValue(KEY_MOVIE_SET_ARTWORK_DIRECTORY, m_movieSetArtworkDirectory.toString());

    QList<QVariant> columns;
    for (const MediaStatusColumn& column : m_mediaStatusColumns) {
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

    settings()->setValue(KEY_DOWNLOADS_DELETE_ARCHIVES, m_deleteArchives);
    settings()->setValue(KEY_DOWNLOADS_KEEP_SOURCE, m_keepDownloadSource);

    settings()->setValue(KEY_TV_SHOWS_SHOW_MISSING_EPISODES, m_showMissingEpisodesHint);

    settings()->setValue(KEY_MOVIES_MULTI_SCRAPE_ONLY_WITH_ID, m_multiScrapeOnlyWithId);
    settings()->setValue(KEY_MOVIES_MULTI_SCRAPE_SAVE_EACH, m_multiScrapeSaveEach);

    settings()->setValue(KEY_MUSIC_ARTISTS_EXTRA_FANARTS, m_extraFanartsMusicArtists);

    settings()->sync();

    emit sigSettingsSaved();
}

/*** GETTER ***/

/**
 * \brief Returns the stored size of the main window
 * \return Size of the main window
 */
QSize Settings::mainWindowSize()
{
    return m_mainWindowSize;
}

/**
 * \brief Returns the stored position of the main window
 * \return Position of the main window
 */
QPoint Settings::mainWindowPosition()
{
    return m_mainWindowPosition;
}

QSize Settings::settingsWindowSize()
{
    return m_settingsWindowSize;
}

QPoint Settings::settingsWindowPosition()
{
    return m_settingsWindowPosition;
}

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

bool Settings::mainWindowMaximized() const
{
    return m_mainWindowMaximized;
}

/**
 * \brief Returns the state of the main splitter
 * \return State of the main splitter
 */
QByteArray Settings::mainSplitterState()
{
    return m_mainSplitterState;
}

QByteArray Settings::movieDuplicatesSplitterState()
{
    return m_movieDuplicatesSplitterState;
}

DirectorySettings& Settings::directorySettings()
{
    return m_directorySettings;
}

KodiSettings& Settings::kodiSettings()
{
    return m_kodiSettings;
}

ImportSettings& Settings::importSettings()
{
    return m_importSettings;
}

NetworkSettings& Settings::networkSettings()
{
    return m_networkSettings;
}

QString Settings::csvExportSeparator()
{
    return m_csvExportSeparator;
}

QString Settings::csvExportReplacement()
{
    return m_csvExportReplacement;
}

QStringList Settings::csvExportTypes()
{
    return m_csvExportTypes;
}

QStringList Settings::csvExportMovieFields()
{
    return m_csvExportMovieFields;
}

QStringList Settings::csvExportTvShowFields()
{
    return m_csvExportTvShowFields;
}

QStringList Settings::csvExportTvEpisodeFields()
{
    return m_csvExportTvEpisodeFields;
}

QStringList Settings::csvExportConcertFields()
{
    return m_csvExportConcertFields;
}

QStringList Settings::csvExportMusicArtistFields()
{
    return m_csvExportMusicArtistFields;
}

QStringList Settings::csvExportMusicAlbumFields()
{
    return m_csvExportMusicAlbumFields;
}

/**
 * \brief Returns the words to exclude from media names,
 * seperated by commas
 * \return exclude words
 */
QStringList Settings::excludeWords()
{
    return m_excludeWords;
}

/**
 * \brief Returns the state of the debug mode
 * \return Debug mode active or not
 */
bool Settings::debugModeActivated() const
{
    return m_debugModeActivated;
}

/**
 * \brief Returns true if urls youtube trailers should be converted
 * \return Change format of URLs to youtube
 */
bool Settings::useYoutubePluginUrls() const
{
    return m_youtubePluginUrls;
}

bool Settings::autoLoadStreamDetails() const
{
    return m_autoLoadStreamDetails;
}

QVector<DataFile> Settings::dataFiles(DataFileType dataType)
{
    QVector<DataFile> files;
    for (const DataFile& file : m_dataFiles) {
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
    for (const DataFile& file : m_initialDataFilesFrodo) {
        if (file.type() == type) {
            files.append(file);
        }
    }
    std::sort(files.begin(), files.end(), DataFile::lessThan);
    return files;
}

bool Settings::usePlotForOutline() const
{
    return m_usePlotForOutline;
}

bool Settings::ignoreDuplicateOriginalTitle() const
{
    return m_ignoreDuplicateOriginalTitle;
}

/*** SETTER ***/

/**
 * \brief Sets the size of the main window
 * \param mainWindowSize Size of the main window
 */
void Settings::setMainWindowSize(QSize mainWindowSize)
{
    m_mainWindowSize = mainWindowSize;
    settings()->setValue(KEY_MAIN_WINDOW_SIZE, mainWindowSize);
}

/**
 * \brief Sets the position of the main window
 * \param mainWindowPosition Position of the main window
 */
void Settings::setMainWindowPosition(QPoint mainWindowPosition)
{
    m_mainWindowPosition = mainWindowPosition;
    settings()->setValue(KEY_MAIN_WINDOW_POSITION, mainWindowPosition);
}

void Settings::setSettingsWindowSize(QSize settingsWindowSize)
{
    m_settingsWindowSize = settingsWindowSize;
    settings()->setValue(KEY_SETTINGS_WINDOW_SIZE, settingsWindowSize);
}

void Settings::setSettingsWindowPosition(QPoint settingsWindowPosition)
{
    m_settingsWindowPosition = settingsWindowPosition;
    settings()->setValue(KEY_SETTINGS_WINDOW_POSITION, settingsWindowPosition);
}

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

void Settings::setMainWindowMaximized(bool max)
{
    m_mainWindowMaximized = max;
    settings()->setValue(KEY_IS_MAIN_WINDOW_MAXIMIZED, max);
}

/**
 * \brief Sets the state of the main splitter
 * \param state State of the splitter
 */
void Settings::setMainSplitterState(QByteArray state)
{
    m_mainSplitterState = state;
    settings()->setValue(KEY_MAIN_SPLITTER_STATE, state);
}

void Settings::setMovieDuplicatesSplitterState(QByteArray state)
{
    m_movieDuplicatesSplitterState = state;
    settings()->setValue(KEY_MOVIE_DUPLICATES_SPLITTER_STATE, state);
}

void Settings::setCsvExportSeparator(QString separator)
{
    m_csvExportSeparator = separator;
    settings()->setValue(KEY_CSV_EXPORT_SEPARATOR, separator);
}

void Settings::setCsvExportReplacement(QString replacement)
{
    m_csvExportReplacement = replacement;
    settings()->setValue(KEY_CSV_EXPORT_REPLACEMENT, replacement);
}

void Settings::setCsvExportTypes(QStringList exportTypes)
{
    m_csvExportTypes = exportTypes;
    settings()->setValue(KEY_CSV_EXPORT_TYPES, exportTypes.join(","));
}

void Settings::setCsvExportMovieFields(QStringList fields)
{
    m_csvExportMovieFields = fields;
    settings()->setValue(KEY_CSV_EXPORT_MOVIE_FIELDS, fields.join(","));
}

void Settings::setCsvExportTvShowFields(QStringList fields)
{
    m_csvExportTvShowFields = fields;
    settings()->setValue(KEY_CSV_EXPORT_TV_SHOW_FIELDS, fields.join(","));
}

void Settings::setCsvExportTvEpisodeFields(QStringList fields)
{
    m_csvExportTvEpisodeFields = fields;
    settings()->setValue(KEY_CSV_EXPORT_TV_EPISODE_FIELDS, fields.join(","));
}

void Settings::setCsvExportConcertFields(QStringList fields)
{
    m_csvExportConcertFields = fields;
    settings()->setValue(KEY_CSV_EXPORT_CONCERT_FIELDS, fields.join(","));
}

void Settings::setCsvExportMusicArtistFields(QStringList fields)
{
    m_csvExportMusicArtistFields = fields;
    settings()->setValue(KEY_CSV_EXPORT_MUSIC_ARTIST_FIELDS, fields.join(","));
}

void Settings::setCsvExportMusicAlbumFields(QStringList fields)
{
    m_csvExportMusicAlbumFields = fields;
    settings()->setValue(KEY_CSV_EXPORT_MUSIC_ALBUM_FIELDS, fields.join(","));
}

/**
 * \brief Sets the exclude words
 * \param words Words to exclude from media names,
 * seperated by commas
 */
void Settings::setExcludeWords(QString words)
{
    m_excludeWords = words.remove(" ").split(",", ElchSplitBehavior::SkipEmptyParts);
}

void Settings::setDebugModeActivated(bool enabled)
{
    m_debugModeActivated = enabled;
}

/**
 * \brief Convert trailer urls to youtube plugin format
 */
void Settings::setUseYoutubePluginUrls(bool use)
{
    m_youtubePluginUrls = use;
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
    m_autoLoadStreamDetails = autoLoad;
}

void Settings::setUsePlotForOutline(bool use)
{
    m_usePlotForOutline = use;
}

void Settings::setIgnoreDuplicateOriginalTitle(bool ignoreDuplicateOriginalTitle)
{
    m_ignoreDuplicateOriginalTitle = ignoreDuplicateOriginalTitle;
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
    return m_downloadActorImages;
}

void Settings::setDownloadActorImages(bool download)
{
    m_downloadActorImages = download;
}

void Settings::renamePatterns(Renamer::RenameType renameType,
    QString& fileNamePattern,
    QString& fileNamePatternMulti,
    QString& directoryPattern,
    QString& seasonPattern)
{
    const QString renameTypeStr = Renamer::typeToString(renameType);
    QString fileNamePatternDefault = "<title>.<extension>";
    QString fileNamePatternMultiDefault = "<title>-part<partNo>.<extension>";
    if (renameType == Renamer::RenameType::TvShows) {
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
}

void Settings::setRenamePatterns(Renamer::RenameType renameType,
    QString fileNamePattern,
    QString fileNamePatternMulti,
    QString directoryPattern,
    QString seasonPattern)
{
    const QString renameTypeStr = Renamer::typeToString(renameType);
    settings()->setValue(QString("RenamePattern/%1/FileName").arg(renameTypeStr), fileNamePattern);
    settings()->setValue(QString("RenamePattern/%1/FileNameMulti").arg(renameTypeStr), fileNamePatternMulti);
    settings()->setValue(QString("RenamePattern/%1/DirectoryPattern").arg(renameTypeStr), directoryPattern);
    settings()->setValue(QString("RenamePattern/%1/SeasonPattern").arg(renameTypeStr), seasonPattern);
}

void Settings::setRenamings(Renamer::RenameType renameType, bool files, bool folders, bool seasonDirectories)
{
    const QString renameTypeStr = Renamer::typeToString(renameType);
    settings()->setValue(QString("RenamePattern/%1/RenameFiles").arg(renameTypeStr), files);
    settings()->setValue(QString("RenamePattern/%1/RenameFolders").arg(renameTypeStr), folders);
    settings()->setValue(QString("RenamePattern/%1/UseSeasonDirectories").arg(renameTypeStr), seasonDirectories);
}

void Settings::renamings(Renamer::RenameType renameType, bool& files, bool& folders, bool& seasonDirectories)
{
    const QString renameTypeStr = Renamer::typeToString(renameType);
    files = settings()->value(QString("RenamePattern/%1/RenameFiles").arg(renameTypeStr), true).toBool();
    folders = settings()->value(QString("RenamePattern/%1/RenameFolders").arg(renameTypeStr), true).toBool();
    seasonDirectories =
        settings()->value(QString("RenamePattern/%1/UseSeasonDirectories").arg(renameTypeStr), true).toBool();
}

int Settings::tvShowUpdateOption()
{
    return settings()->value(KEY_TV_SHOW_UPDATE_OPTION, 0).toInt();
}

void Settings::setTvShowUpdateOption(int option)
{
    settings()->setValue(KEY_TV_SHOW_UPDATE_OPTION, option);
}

AdvancedSettings* Settings::advanced()
{
    return &m_advancedSettings;
}

bool Settings::ignoreArticlesWhenSorting() const
{
    return m_ignoreArticlesWhenSorting;
}

void Settings::setIgnoreArticlesWhenSorting(bool ignore)
{
    m_ignoreArticlesWhenSorting = ignore;
}

void Settings::setMovieSetArtworkType(MovieSetArtworkType type)
{
    m_movieSetArtworkType = type;
}

MovieSetArtworkType Settings::movieSetArtworkType() const
{
    return m_movieSetArtworkType;
}

void Settings::setMovieSetArtworkDirectory(mediaelch::DirectoryPath dir)
{
    m_movieSetArtworkDirectory = dir;
}

mediaelch::DirectoryPath Settings::movieSetArtworkDirectory() const
{
    return m_movieSetArtworkDirectory;
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
    return m_seasonOrder;
}

void Settings::setSeasonOrder(SeasonOrder order)
{
    m_seasonOrder = order;
    saveSettings();
}

void Settings::setDontShowDeleteImageConfirm(bool show)
{
    m_dontShowDeleteImageConfirm = show;
    saveSettings();
}

bool Settings::dontShowDeleteImageConfirm() const
{
    return m_dontShowDeleteImageConfirm;
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
    // Ensure that all details are set. Default is TheTvDb because it supports most.
    for (const ShowScraperInfo info : mediaelch::allShowScraperInfos()) {
        if (!customTvScraper.contains(info)) {
            customTvScraper.insert(info, mediaelch::scraper::TheTvDb::ID);
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
    // Ensure that all details are set. Default is TheTvDb because it supports most.
    for (const EpisodeScraperInfo info : mediaelch::allEpisodeScraperInfos()) {
        if (!customTvScraper.contains(info)) {
            customTvScraper.insert(info, mediaelch::scraper::TheTvDb::ID);
        }
    }
    m_customTvScraperEpisode = customTvScraper;
}

int Settings::currentMovieScraper() const
{
    return m_currentMovieScraper;
}

void Settings::setCurrentMovieScraper(int current)
{
    m_currentMovieScraper = current;
    settings()->setValue(KEY_SCRAPER_CURRENT_MOVIE_SCRAPER, current);
    settings()->sync();
}

const QString& Settings::currentTvShowScraper() const
{
    return m_currentTvShowScraper;
}

const QString& Settings::currentConcertScraper() const
{
    return m_currentConcertScraper;
}

void Settings::setCurrentTvShowScraper(const QString& current)
{
    m_currentTvShowScraper = current;
    settings()->setValue(KEY_SCRAPER_CURRENT_TV_SHOW_SCRAPER, current);
    settings()->sync();
}

void Settings::setCurrentConcertScraper(const QString& current)
{
    m_currentConcertScraper = current;
    settings()->setValue(KEY_SCRAPER_CURRENT_CONCERT_SCRAPER, current);
    settings()->sync();
}

void Settings::setDeleteArchives(bool deleteArchives)
{
    m_deleteArchives = deleteArchives;
}

bool Settings::deleteArchives() const
{
    return m_deleteArchives;
}

void Settings::setKeepDownloadSource(bool keep)
{
    m_keepDownloadSource = keep;
}

bool Settings::keepDownloadSource() const
{
    return m_keepDownloadSource;
}

void Settings::setCheckForUpdates(bool check)
{
    m_checkForUpdates = check;
}

bool Settings::checkForUpdates() const
{
    return m_checkForUpdates;
}

void Settings::setShowMissingEpisodesHint(bool show)
{
    m_showMissingEpisodesHint = show;
}

bool Settings::showMissingEpisodesHint() const
{
    return m_showMissingEpisodesHint;
}

void Settings::setMultiScrapeOnlyWithId(bool onlyWithId)
{
    m_multiScrapeOnlyWithId = onlyWithId;
}

bool Settings::multiScrapeOnlyWithId() const
{
    return m_multiScrapeOnlyWithId;
}

void Settings::setMultiScrapeSaveEach(bool saveEach)
{
    m_multiScrapeSaveEach = saveEach;
}

bool Settings::multiScrapeSaveEach() const
{
    return m_multiScrapeSaveEach;
}

QString Settings::applicationDir()
{
    return QApplication::applicationDirPath();
}

mediaelch::DirectoryPath Settings::databaseDir()
{
    if (advanced()->portableMode()) {
        return applicationDir();
    }
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

mediaelch::DirectoryPath Settings::imageCacheDir()
{
    if (advanced()->portableMode()) {
        return applicationDir();
    }
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

mediaelch::DirectoryPath Settings::exportTemplatesDir()
{
    if (advanced()->portableMode()) {
        return applicationDir() + QDir::separator() + "export_themes";
    }
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "export_themes";
}

void Settings::setShowAdultScrapers(bool show)
{
    m_showAdultScrapers = show;
}

bool Settings::showAdultScrapers() const
{
    return m_showAdultScrapers;
}

void Settings::setStartupSection(QString startupSection)
{
    m_startupSection = startupSection;
}

QString Settings::startupSection()
{
    return m_startupSection;
}

void Settings::setDonated(bool donated)
{
    m_donated = donated;
    settings()->setValue(KEY_DONATED, m_donated);
    settings()->sync();
    emit sigDonated(donated);
}

bool Settings::donated() const
{
    return m_donated;
}

void Settings::setLastImagePath(mediaelch::DirectoryPath path)
{
    m_lastImagePath = path;
    settings()->sync();
}

mediaelch::DirectoryPath Settings::lastImagePath()
{
    return m_lastImagePath;
}

QPoint Settings::fixWindowPosition(QPoint p)
{
    p.setX(qMax(0, p.x()));
    p.setY(qMax(0, p.y()));
    return p;
}


int Settings::extraFanartsMusicArtists() const
{
    return m_extraFanartsMusicArtists;
}

void Settings::setExtraFanartsMusicArtists(int extraFanartsMusicArtists)
{
    m_extraFanartsMusicArtists = extraFanartsMusicArtists;
}
