#include "Settings.h"

#include "globals/Manager.h"
#include "renamer/RenamerDialog.h"
#include "scrapers/movie/MovieScraperInterface.h"
#include "settings/AdvancedSettingsXmlReader.h"

#include <QApplication>
#include <QDesktopServices>
#include <QMutex>
#include <QMutexLocker>

Settings::Settings(QObject* parent) : QObject(parent)
{
    auto advancedSettingsPair = AdvancedSettingsXmlReader::loadFromDefaultPath();
    m_advancedSettings = advancedSettingsPair.first;

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
 * @brief Returns an instance of the settings
 * @param parent Parent widget
 * @return Instance of Settings
 */
Settings* Settings::instance(QObject* parent)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    static Settings* m_instance = nullptr;
    if (m_instance == nullptr) {
        m_instance = new Settings(parent);
    }
    return m_instance;
}

QSettings* Settings::settings()
{
    return m_settings;
}

/**
 * @brief Loads all settings
 */
void Settings::loadSettings()
{
    // Globals
    m_mainWindowSize = settings()->value("MainWindowSize").toSize();
    m_settingsWindowSize = settings()->value("SettingsWindowSize").toSize();
    m_mainWindowMaximized = settings()->value("MainWindowMaximized").toBool();
    m_mainSplitterState = settings()->value("MainSplitterState").toByteArray();
    m_movieDuplicatesSplitterState = settings()->value("MovieDuplicatesSplitterState").toByteArray();
    m_debugModeActivated = settings()->value("DebugModeActivated", false).toBool();
    m_debugLogPath = settings()->value("DebugLogPath").toString();
    m_autoLoadStreamDetails = settings()->value("AutoLoadStreamDetails", true).toBool();
    m_usePlotForOutline = settings()->value("Movies/UsePlotForOutline", true).toBool();
    m_downloadActorImages = settings()->value("DownloadActorImages", true).toBool();
    m_ignoreArticlesWhenSorting = settings()->value("IgnoreArticlesWhenSorting", false).toBool();
    m_checkForUpdates = settings()->value("CheckForUpdates", true).toBool();
    m_showAdultScrapers = settings()->value("Scrapers/ShowAdult", false).toBool();
    m_startupSection = settings()->value("StartupSection", "movies").toString();
    m_donated = settings()->value("Donated", false).toBool();
    m_lastImagePath = settings()->value("LastImagePath", QDir::homePath()).toString();

    // Window positions
    m_mainWindowPosition = fixWindowPosition(settings()->value("MainWindowPosition").toPoint());
    m_settingsWindowPosition = fixWindowPosition(settings()->value("SettingsWindowPosition").toPoint());
    m_importDialogPosition = fixWindowPosition(settings()->value("Downloads/ImportDialogPosition").toPoint());
    m_makeMkvDialogPosition = fixWindowPosition(settings()->value("Downloads/MakeMkvDialogPosition").toPoint());

    // Tv Shows
    m_tvShowDvdOrder = settings()->value("TvShows/DvdOrder", false).toBool();

    // Warnings
    m_dontShowDeleteImageConfirm = settings()->value("Warnings/DontShowDeleteImageConfirm", false).toBool();

    m_directorySettings.loadSettings();
    m_kodiSettings.loadSettings();
    m_importSettings.loadSettings();
    m_networkSettings.loadSettings();

    m_excludeWords = settings()->value("excludeWords").toString();
    if (m_excludeWords.isEmpty()) {
        m_excludeWords = "ac3,dts,custom,dc,divx,divx5,dsr,dsrip,dutch,dvd,dvdrip,dvdscr,dvdscreener,screener,dvdivx,"
                         "cam,fragment,fs,hdtv,hdrip,hdtvrip,internal,limited,"
                         "multisubs,ntsc,ogg,ogm,pal,pdtv,proper,repack,rerip,retail,r3,r5,bd5,se,svcd,swedish,german,"
                         "read.nfo,nfofix,unrated,ws,telesync,ts,telecine,tc,"
                         "brrip,bdrip,480p,480i,576p,576i,720p,720i,1080p,1080i,hrhd,hrhdtv,hddvd,bluray,x264,h264,"
                         "xvid,xvidvd,xxx,www,mkv";
    }

    const auto loadSettings = [&](auto scrapers) {
        for (auto* scraper : scrapers) {
            if (scraper->hasSettings()) {
                ScraperSettingsQt scraperSettings(*scraper, *m_settings);
                scraper->loadSettings(scraperSettings);
            }
        }
    };
    loadSettings(Manager::instance()->movieScrapers());
    loadSettings(Manager::instance()->tvScrapers());
    loadSettings(Manager::instance()->concertScrapers());
    loadSettings(Manager::instance()->musicScrapers());
    loadSettings(Manager::instance()->imageProviders());

    m_currentMovieScraper = settings()->value("Scraper/CurrentMovieScraper", 0).toInt();

    // Media Centers
    m_youtubePluginUrls = settings()->value("UseYoutubePluginURLs", false).toBool();

    // Data Files
    QVector<DataFile> dataFiles;
    int dataFileSize = settings()->beginReadArray("AllDataFiles");
    for (int i = 0; i < dataFileSize; ++i) {
        settings()->setArrayIndex(i);
        DataFileType type = DataFileType(settings()->value("type").toInt());
        QString fileName = settings()->value("fileName").toString();
        if (fileName.isEmpty()) {
            for (const DataFile& initialDataFile : m_initialDataFilesFrodo) {
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

    for (const DataFile& initialDataFile : m_initialDataFilesFrodo) {
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
    m_movieSetArtworkType = MovieSetArtworkType(settings()->value("MovieSetArtwork/StoringType", 0).toInt());
    m_movieSetArtworkDirectory = settings()->value("MovieSetArtwork/Directory").toString();

    // Media Status Columns
    m_mediaStatusColumns.clear();
    for (const QVariant& column : settings()->value("MediaStatusColumn").toList()) {
        m_mediaStatusColumns.append(static_cast<MediaStatusColumn>(column.toInt()));
    }


    m_customMovieScraper.clear();
    int customMovieScraperSize = settings()->beginReadArray("CustomMovieScraper");
    for (int i = 0; i < customMovieScraperSize; ++i) {
        settings()->setArrayIndex(i);
        m_customMovieScraper.insert(
            MovieScraperInfos(settings()->value("Info").toInt()), settings()->value("Scraper").toString());
    }
    settings()->endArray();

    m_customTvScraper.clear();
    int customTvScraperSize = settings()->beginReadArray("CustomTvScraper");
    for (int i = 0; i < customTvScraperSize; ++i) {
        settings()->setArrayIndex(i);
        m_customTvScraper.insert(
            TvShowScraperInfos(settings()->value("Info").toInt()), settings()->value("Scraper").toString());
    }
    settings()->endArray();

    // Downloads
    m_deleteArchives = settings()->value("Downloads/DeleteArchives", false).toBool();
    m_importDialogSize = settings()->value("Downloads/ImportDialogSize").toSize();
    m_makeMkvDialogSize = settings()->value("Downloads/MakeMkvDialogSize").toSize();
    m_keepDownloadSource = settings()->value("Downloads/KeepSource", true).toBool();

    // Movies
    m_multiScrapeOnlyWithId = settings()->value("Movies/MultiScrapeOnlyWithId", false).toBool();
    m_multiScrapeSaveEach = settings()->value("Movies/MultiScrapeSaveEach", false).toBool();

    m_showMissingEpisodesHint = settings()->value("TvShows/ShowMissingEpisodesHint", true).toBool();

    m_extraFanartsMusicArtists = settings()->value("Music/Artists/ExtraFanarts", 0).toInt();
}

/**
 * @brief Saves all settings
 */
void Settings::saveSettings()
{
    settings()->setValue("DebugModeActivated", m_debugModeActivated);
    settings()->setValue("DebugLogPath", m_debugLogPath);
    settings()->setValue("AutoLoadStreamDetails", m_autoLoadStreamDetails);

    settings()->setValue("UseYoutubePluginURLs", m_youtubePluginUrls);
    settings()->setValue("Movies/UsePlotForOutline", m_usePlotForOutline);
    settings()->setValue("DownloadActorImages", m_downloadActorImages);
    settings()->setValue("IgnoreArticlesWhenSorting", m_ignoreArticlesWhenSorting);
    settings()->setValue("CheckForUpdates", m_checkForUpdates);
    settings()->setValue("Scrapers/ShowAdult", m_showAdultScrapers);
    settings()->setValue("StartupSection", m_startupSection);
    settings()->setValue("Donated", m_donated);
    settings()->setValue("LastImagePath", m_lastImagePath);


    // Tv Shows
    settings()->setValue("TvShows/DvdOrder", m_tvShowDvdOrder);

    // Warnings
    settings()->setValue("Warnings/DontShowDeleteImageConfirm", m_dontShowDeleteImageConfirm);

    m_directorySettings.saveSettings();
    m_kodiSettings.saveSettings();
    m_importSettings.saveSettings();
    m_networkSettings.saveSettings();

    settings()->setValue("excludeWords", m_excludeWords);

    const auto saveSettings = [&](auto scrapers) {
        for (auto* scraper : scrapers) {
            if (scraper->hasSettings()) {
                ScraperSettingsQt scraperSettings(*scraper, *m_settings);
                scraper->saveSettings(scraperSettings);
            }
        }
    };
    saveSettings(Manager::instance()->movieScrapers());
    saveSettings(Manager::instance()->tvScrapers());
    saveSettings(Manager::instance()->concertScrapers());
    saveSettings(Manager::instance()->musicScrapers());
    saveSettings(Manager::instance()->imageProviders());

    settings()->setValue("Scraper/CurrentMovieScraper", m_currentMovieScraper);

    settings()->beginWriteArray("AllDataFiles");
    for (int i = 0, n = m_dataFiles.count(); i < n; ++i) {
        settings()->setArrayIndex(i);
        settings()->setValue("type", static_cast<int>(m_dataFiles.at(i).type()));
        settings()->setValue("fileName", m_dataFiles.at(i).fileName());
        settings()->setValue("pos", m_dataFiles.at(i).pos());
    }
    settings()->endArray();

    settings()->setValue("MovieSetArtwork/StoringType", static_cast<int>(m_movieSetArtworkType));
    settings()->setValue("MovieSetArtwork/Directory", m_movieSetArtworkDirectory);

    QList<QVariant> columns;
    for (const MediaStatusColumn& column : m_mediaStatusColumns) {
        columns.append(static_cast<int>(column));
    }
    settings()->setValue("MediaStatusColumn", columns);

    int i = 0;
    settings()->beginWriteArray("CustomMovieScraper");
    QMapIterator<MovieScraperInfos, QString> it(m_customMovieScraper);
    while (it.hasNext()) {
        it.next();
        settings()->setArrayIndex(i++);
        settings()->setValue("Info", static_cast<int>(it.key()));
        settings()->setValue("Scraper", it.value());
    }
    settings()->endArray();

    i = 0;
    settings()->beginWriteArray("CustomTvScraper");
    QMapIterator<TvShowScraperInfos, QString> itTv(m_customTvScraper);
    while (itTv.hasNext()) {
        itTv.next();
        settings()->setArrayIndex(i++);
        settings()->setValue("Info", static_cast<int>(itTv.key()));
        settings()->setValue("Scraper", itTv.value());
    }
    settings()->endArray();

    settings()->setValue("Downloads/DeleteArchives", m_deleteArchives);
    settings()->setValue("Downloads/KeepSource", m_keepDownloadSource);

    settings()->setValue("TvShows/ShowMissingEpisodesHint", m_showMissingEpisodesHint);

    settings()->setValue("Movies/MultiScrapeOnlyWithId", m_multiScrapeOnlyWithId);
    settings()->setValue("Movies/MultiScrapeSaveEach", m_multiScrapeSaveEach);

    settings()->setValue("Music/Artists/ExtraFanarts", m_extraFanartsMusicArtists);

    settings()->sync();

    emit sigSettingsSaved();
}

/*** GETTER ***/

/**
 * @brief Returns the stored size of the main window
 * @return Size of the main window
 */
QSize Settings::mainWindowSize()
{
    return m_mainWindowSize;
}

/**
 * @brief Returns the stored position of the main window
 * @return Position of the main window
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

bool Settings::mainWindowMaximized()
{
    return m_mainWindowMaximized;
}

/**
 * @brief Returns the state of the main splitter
 * @return State of the main splitter
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

/**
 * @brief Returns the words to exclude from media names,
 * seperated by commas
 * @return exclude words
 */
QString Settings::excludeWords()
{
    return m_excludeWords;
}

/**
 * @brief Returns the state of the debug mode
 * @return Debug mode active or not
 */
bool Settings::debugModeActivated()
{
    return m_debugModeActivated;
}

/**
 * @brief Returns the path to the logfile
 * @return Path to logfile
 */
QString Settings::debugLogPath()
{
    return m_debugLogPath;
}

/**
 * @brief Returns true if urls youtube trailers should be converted
 * @return Change format of URLs to youtube
 */
bool Settings::useYoutubePluginUrls()
{
    return m_youtubePluginUrls;
}

bool Settings::autoLoadStreamDetails()
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

bool Settings::usePlotForOutline()
{
    return m_usePlotForOutline;
}

/*** SETTER ***/

/**
 * @brief Sets the size of the main window
 * @param mainWindowSize Size of the main window
 */
void Settings::setMainWindowSize(QSize mainWindowSize)
{
    m_mainWindowSize = mainWindowSize;
    settings()->setValue("MainWindowSize", mainWindowSize);
}

/**
 * @brief Sets the position of the main window
 * @param mainWindowPosition Position of the main window
 */
void Settings::setMainWindowPosition(QPoint mainWindowPosition)
{
    m_mainWindowPosition = mainWindowPosition;
    settings()->setValue("MainWindowPosition", mainWindowPosition);
}

void Settings::setSettingsWindowSize(QSize settingsWindowSize)
{
    m_settingsWindowSize = settingsWindowSize;
    settings()->setValue("SettingsWindowSize", settingsWindowSize);
}

void Settings::setSettingsWindowPosition(QPoint settingsWindowPosition)
{
    m_settingsWindowPosition = settingsWindowPosition;
    settings()->setValue("SettingsWindowPosition", settingsWindowPosition);
}

void Settings::setImportDialogSize(QSize size)
{
    m_importDialogSize = size;
    settings()->setValue("Downloads/ImportDialogSize", size);
}

void Settings::setImportDialogPosition(QPoint position)
{
    m_importDialogPosition = position;
    settings()->setValue("Downloads/ImportDialogPosition", position);
}

void Settings::setMakeMkvDialogSize(QSize size)
{
    m_makeMkvDialogSize = size;
    settings()->setValue("Downloads/MakeMkvDialogSize", size);
}

void Settings::setMakeMkvDialogPosition(QPoint position)
{
    m_makeMkvDialogPosition = position;
    settings()->setValue("Downloads/MakeMkvDialogPosition", position);
}

void Settings::setMainWindowMaximized(bool max)
{
    m_mainWindowMaximized = max;
    settings()->setValue("MainWindowMaximized", max);
}

/**
 * @brief Sets the state of the main splitter
 * @param state State of the splitter
 */
void Settings::setMainSplitterState(QByteArray state)
{
    m_mainSplitterState = state;
    settings()->setValue("MainSplitterState", state);
}

void Settings::setMovieDuplicatesSplitterState(QByteArray state)
{
    m_movieDuplicatesSplitterState = state;
    settings()->setValue("MovieDuplicatesSplitterState", state);
}

/**
 * @brief Sets the exclude words
 * @param words Words to exclude from media names,
 * seperated by commas
 */
void Settings::setExcludeWords(QString words)
{
    m_excludeWords = words;
}

void Settings::setDebugModeActivated(bool enabled)
{
    m_debugModeActivated = enabled;
}

/**
 * @brief Sets the path to the logfile
 * @param path Path to log file
 */
void Settings::setDebugLogPath(QString path)
{
    m_debugLogPath = path;
}

/**
 * @brief Convert trailer urls to youtube plugin format
 */
void Settings::setUseYoutubePluginUrls(bool use)
{
    m_youtubePluginUrls = use;
}

/**
 * @brief Settings::setDataFiles
 */
void Settings::setDataFiles(QVector<DataFile> files)
{
    m_dataFiles = files;
}

/**
 * @brief Settings::setAutoLoadStreamDetails
 */
void Settings::setAutoLoadStreamDetails(bool autoLoad)
{
    m_autoLoadStreamDetails = autoLoad;
}

/**
 * @brief Settings::setUsePlotForOutline
 */
void Settings::setUsePlotForOutline(bool use)
{
    m_usePlotForOutline = use;
}

template<>
QVector<ConcertScraperInfos> Settings::scraperInfos(QString scraperId)
{
    QVector<ConcertScraperInfos> infos;
    for (const auto& info : settings()->value(QString("Scrapers/Movies/%1").arg(scraperId)).toString().split(",")) {
        infos << ConcertScraperInfos(info.toInt());
    }
    if (!infos.isEmpty() && static_cast<int>(infos.first()) == 0) {
        infos.clear();
    }
    return infos;
}

template<>
QVector<MovieScraperInfos> Settings::scraperInfos(QString scraperId)
{
    QVector<MovieScraperInfos> infos;
    for (const auto& info : settings()->value(QString("Scrapers/Movies/%1").arg(scraperId)).toString().split(",")) {
        infos << MovieScraperInfos(info.toInt());
    }
    if (!infos.isEmpty() && static_cast<int>(infos.first()) == 0) {
        infos.clear();
    }
    return infos;
}

template<>
QVector<TvShowScraperInfos> Settings::scraperInfos(QString scraperId)
{
    QVector<TvShowScraperInfos> infos;
    for (const auto& info : settings()->value(QString("Scrapers/TvShows/%1").arg(scraperId)).toString().split(",")) {
        infos << TvShowScraperInfos(info.toInt());
    }
    if (!infos.isEmpty() && static_cast<int>(infos.first()) == 0) {
        infos.clear();
    }
    return infos;
}

template<>
QVector<MusicScraperInfos> Settings::scraperInfos(QString scraperId)
{
    QVector<MusicScraperInfos> infos;
    for (const auto& info : settings()->value(QString("Scrapers/Music/%1").arg(scraperId)).toString().split(",")) {
        infos << MusicScraperInfos(info.toInt());
    }
    if (!infos.isEmpty() && static_cast<int>(infos.first()) == 0) {
        infos.clear();
    }
    return infos;
}


void Settings::setScraperInfos(MainWidgets widget, QString scraperNo, QVector<MovieScraperInfos> items)
{
    Q_UNUSED(widget);
    QStringList infos;
    infos.reserve(items.size());
    for (const auto info : items) {
        infos << QString::number(static_cast<int>(info));
    }
    settings()->setValue(QString("Scrapers/Movies/%2").arg(scraperNo), infos.join(","));
}

void Settings::setScraperInfos(MainWidgets widget, QString scraperNo, QVector<TvShowScraperInfos> items)
{
    Q_UNUSED(widget);
    QStringList infos;
    infos.reserve(items.size());
    for (const auto info : items) {
        infos << QString::number(static_cast<int>(info));
    }
    settings()->setValue(QString("Scrapers/TvShows/%2").arg(scraperNo), infos.join(","));
}

void Settings::setScraperInfos(MainWidgets widget, QString scraperNo, QVector<ConcertScraperInfos> items)
{
    Q_UNUSED(widget);
    QStringList infos;
    infos.reserve(items.size());
    for (const auto info : items) {
        infos << QString::number(static_cast<int>(info));
    }
    settings()->setValue(QString("Scrapers/Concerts/%2").arg(scraperNo), infos.join(","));
}

void Settings::setScraperInfos(MainWidgets widget, QString scraperNo, QVector<MusicScraperInfos> items)
{
    Q_UNUSED(widget);
    QStringList infos;
    infos.reserve(items.size());
    for (const auto info : items) {
        infos << QString::number(static_cast<int>(info));
    }
    settings()->setValue(QString("Scrapers/Music/%2").arg(scraperNo), infos.join(","));
}


bool Settings::downloadActorImages()
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
    return settings()->value("TvShowUpdateOption", 0).toInt();
}

void Settings::setTvShowUpdateOption(int option)
{
    settings()->setValue("TvShowUpdateOption", option);
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

void Settings::setMovieSetArtworkDirectory(QString dir)
{
    m_movieSetArtworkDirectory = dir;
}

QString Settings::movieSetArtworkDirectory() const
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

bool Settings::tvShowDvdOrder() const
{
    return m_tvShowDvdOrder;
}

void Settings::setTvShowDvdOrder(bool order)
{
    m_tvShowDvdOrder = order;
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

QMap<MovieScraperInfos, QString> Settings::customMovieScraper() const
{
    return m_customMovieScraper;
}

void Settings::setCustomMovieScraper(QMap<MovieScraperInfos, QString> customMovieScraper)
{
    m_customMovieScraper = customMovieScraper;
}

QMap<TvShowScraperInfos, QString> Settings::customTvScraper() const
{
    return m_customTvScraper;
}

void Settings::setCustomTvScraper(QMap<TvShowScraperInfos, QString> customTvScraper)
{
    m_customTvScraper = customTvScraper;
}

int Settings::currentMovieScraper() const
{
    return m_currentMovieScraper;
}

void Settings::setCurrentMovieScraper(int current)
{
    m_currentMovieScraper = current;
    settings()->setValue("Scraper/CurrentMovieScraper", current);
    settings()->sync();
}

void Settings::setDeleteArchives(bool deleteArchives)
{
    m_deleteArchives = deleteArchives;
}

bool Settings::deleteArchives()
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

QString Settings::databaseDir()
{
    if (advanced()->portableMode()) {
        return applicationDir();
    }
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

QString Settings::imageCacheDir()
{
    if (advanced()->portableMode()) {
        return applicationDir();
    }
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

QString Settings::exportTemplatesDir()
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
    settings()->setValue("Donated", m_donated);
    settings()->sync();
    emit sigDonated(donated);
}

bool Settings::donated() const
{
    return m_donated;
}

void Settings::setLastImagePath(QString path)
{
    m_lastImagePath = path;
    settings()->sync();
}

QString Settings::lastImagePath()
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
