#include "Settings.h"

#include <QApplication>
#include <QDesktopServices>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkProxy>
#include "data/ScraperInterface.h"
#include "globals/Manager.h"
#include "plugins/PluginManager.h"
#include "renamer/Renamer.h"

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings(QObject *parent) :
    QObject(parent)
{
    m_advancedSettings = new AdvancedSettings(parent);

    if (m_advancedSettings->portableMode()) {
        qDebug() << "portable mode!";
        m_settings = new QSettings(Settings::applicationDir() + "/MediaElch.ini", QSettings::IniFormat, this);
    } else {
        m_settings = new QSettings(this);
    }

    // Frodo
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MoviePoster, "<baseFileName>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieCdArt, "disc.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieClearArt, "clearart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieLogo, "logo.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieBanner, "<baseFileName>-banner.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieThumb, "<baseFileName>-landscape.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieSetPoster, "folder.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieSetBackdrop, "fanart.jpg", 0));

    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowNfo, "tvshow.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowBackdrop, "fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowBanner, "banner.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowCharacterArt, "character.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowClearArt, "clearart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowLogo, "logo.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowPoster, "poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowPoster, "season-all-poster.jpg", 1));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowSeasonPoster, "season<seasonNumber>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowSeasonBackdrop, "season<seasonNumber>-fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowSeasonBanner, "season<seasonNumber>-banner.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowEpisodeNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowEpisodeThumb, "<baseFileName>-thumb.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowThumb, "landscape.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::TvShowSeasonThumb, "season<seasonNumber>-landscape.jpg", 0));

    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertPoster, "<baseFileName>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertCdArt, "disc.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertClearArt, "clearart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertLogo, "logo.png", 0));

    m_initialDataFilesFrodo.append(DataFile(DataFileType::ArtistFanart, "fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ArtistLogo, "logo.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ArtistThumb, "folder.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::AlbumCdArt, "cdart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::AlbumThumb, "folder.jpg", 0));
}

/**
 * @brief Returns an instance of the settings
 * @param parent Parent widget
 * @return Instance of Settings
 */
Settings *Settings::instance(QObject *parent)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    static Settings *m_instance = 0;
    if (!m_instance)
        m_instance = new Settings(parent);
    return m_instance;
}

QSettings *Settings::settings()
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

    // XBMC
    m_xbmcHost = settings()->value("XBMC/RemoteHost").toString();
    m_xbmcPort = settings()->value("XBMC/RemotePort", 80).toInt();
    m_xbmcUser = settings()->value("XBMC/RemoteUser").toString();
    m_xbmcPassword = settings()->value("XBMC/RemotePassword").toString();

    // Proxy
    m_useProxy = settings()->value("Proxy/Enable", false).toBool();
    m_proxyType = settings()->value("Proxy/Type", 0).toInt();
    m_proxyHost = settings()->value("Proxy/Host").toString();
    m_proxyPort = settings()->value("Proxy/Port", 0).toInt();
    m_proxyUsername = settings()->value("Proxy/Username").toString();
    m_proxyPassword = settings()->value("Proxy/Password").toString();
    setupProxy();

    // Tv Shows
    m_tvShowDvdOrder = settings()->value("TvShows/DvdOrder", false).toBool();

    // Warnings
    m_dontShowDeleteImageConfirm = settings()->value("Warnings/DontShowDeleteImageConfirm", false).toBool();

    // Movie Directories
    m_movieDirectories.clear();
    int moviesSize = settings()->beginReadArray("Directories/Movies");
    for (int i=0 ; i<moviesSize ; ++i) {
        settings()->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(settings()->value("path").toString());
        dir.separateFolders = settings()->value("sepFolders", false).toBool();
        dir.autoReload = settings()->value("autoReload", false).toBool();
        m_movieDirectories.append(dir);
    }
    settings()->endArray();

    // TV Show Directories
    m_tvShowDirectories.clear();
    int tvShowSize = settings()->beginReadArray("Directories/TvShows");
    for (int i=0 ; i<tvShowSize ; ++i) {
        settings()->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(settings()->value("path").toString());
        dir.separateFolders = settings()->value("sepFolders", false).toBool();
        dir.autoReload = settings()->value("autoReload", false).toBool();
        m_tvShowDirectories.append(dir);
    }
    settings()->endArray();

    // Concert Directories
    m_concertDirectories.clear();
    int concertsSize = settings()->beginReadArray("Directories/Concerts");
    for (int i=0 ; i<concertsSize ; ++i) {
        settings()->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(settings()->value("path").toString());
        dir.separateFolders = settings()->value("sepFolders", false).toBool();
        dir.autoReload = settings()->value("autoReload", false).toBool();
        m_concertDirectories.append(dir);
    }
    settings()->endArray();

    m_downloadDirectories.clear();
    int downloadsSize = settings()->beginReadArray("Directories/Downloads");
    for (int i=0 ; i<downloadsSize ; ++i) {
        settings()->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(settings()->value("path").toString());
        dir.separateFolders = settings()->value("sepFolders", false).toBool();
        dir.autoReload = settings()->value("autoReload", false).toBool();
        m_downloadDirectories.append(dir);
    }
    settings()->endArray();

    m_musicDirectories.clear();
    int musicSize = settings()->beginReadArray("Directories/Music");
    for (int i=0 ; i<musicSize ; ++i) {
        settings()->setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(settings()->value("path").toString());
        dir.separateFolders = settings()->value("sepFolders", false).toBool();
        dir.autoReload = settings()->value("autoReload", false).toBool();
        m_musicDirectories.append(dir);
    }
    settings()->endArray();

    m_excludeWords = settings()->value("excludeWords").toString();
    if (m_excludeWords.isEmpty())
        m_excludeWords = "ac3,dts,custom,dc,divx,divx5,dsr,dsrip,dutch,dvd,dvdrip,dvdscr,dvdscreener,screener,dvdivx,cam,fragment,fs,hdtv,hdrip,hdtvrip,internal,limited,"
                         "multisubs,ntsc,ogg,ogm,pal,pdtv,proper,repack,rerip,retail,r3,r5,bd5,se,svcd,swedish,german,read.nfo,nfofix,unrated,ws,telesync,ts,telecine,tc,"
                         "brrip,bdrip,480p,480i,576p,576i,720p,720i,1080p,1080i,hrhd,hrhdtv,hddvd,bluray,x264,h264,xvid,xvidvd,xxx,www,mkv";

    // Scrapers
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers())
        scraper->loadSettings(*settings());
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers())
        scraper->loadSettings(*settings());
    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers())
        scraper->loadSettings(*settings());
    foreach (MusicScraperInterface *scraper, Manager::instance()->musicScrapers())
        scraper->loadSettings(*settings());
    foreach (ImageProviderInterface *scraper, Manager::instance()->imageProviders())
        scraper->loadSettings(*settings());

    m_currentMovieScraper = settings()->value("Scraper/CurrentMovieScraper", 0).toInt();

    // Media Centers
    m_youtubePluginUrls = settings()->value("UseYoutubePluginURLs", false).toBool();

    // Data Files
    QList<DataFile> dataFiles;
    int dataFileSize = settings()->beginReadArray("AllDataFiles");
    for (int i=0 ; i<dataFileSize ; ++i) {
        settings()->setArrayIndex(i);
        int type = settings()->value("type").toInt();
        QString fileName = settings()->value("fileName").toString();
        if (fileName.isEmpty()) {
            foreach (DataFile initialDataFile, m_initialDataFilesFrodo) {
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

    foreach (DataFile initialDataFile, m_initialDataFilesFrodo) {
        bool found = false;
        foreach (DataFile df, dataFiles) {
            if (df.type() == initialDataFile.type()) {
                found = true;
                break;
            }

        }
        if (!found)
            dataFiles << initialDataFile;
    }

    if (dataFiles.isEmpty())
        m_dataFiles = m_initialDataFilesFrodo;
    else
        m_dataFiles = dataFiles;

    // Movie set artwork
    m_movieSetArtworkType = settings()->value("MovieSetArtwork/StoringType", 0).toInt();
    m_movieSetArtworkDirectory = settings()->value("MovieSetArtwork/Directory").toString();

    // Media Status Columns
    m_mediaStatusColumns.clear();
    foreach (const QVariant &column, settings()->value("MediaStatusColumns").toList())
        m_mediaStatusColumns.append(static_cast<MediaStatusColumns>(column.toInt()));


    m_customMovieScraper.clear();
    int customMovieScraperSize = settings()->beginReadArray("CustomMovieScraper");
    for (int i=0 ; i<customMovieScraperSize ; ++i) {
        settings()->setArrayIndex(i);
        m_customMovieScraper.insert(settings()->value("Info").toInt(), settings()->value("Scraper").toString());
    }
    settings()->endArray();

    m_customTvScraper.clear();
    int customTvScraperSize = settings()->beginReadArray("CustomTvScraper");
    for (int i=0 ; i<customTvScraperSize ; ++i) {
        settings()->setArrayIndex(i);
        m_customTvScraper.insert(settings()->value("Info").toInt(), settings()->value("Scraper").toString());
    }
    settings()->endArray();

    // Downloads
    m_unrar = settings()->value("Downloads/Unrar").toString();
    m_makeMkvCon = settings()->value("Downloads/MakeMkvCon").toString();
    m_deleteArchives = settings()->value("Downloads/DeleteArchives", false).toBool();
    m_importDialogSize = settings()->value("Downloads/ImportDialogSize").toSize();
    m_makeMkvDialogSize = settings()->value("Downloads/MakeMkvDialogSize").toSize();
    m_keepDownloadSource = settings()->value("Downloads/KeepSource", true).toBool();

    // Movies
    m_multiScrapeOnlyWithId = settings()->value("Movies/MultiScrapeOnlyWithId", false).toBool();
    m_multiScrapeSaveEach = settings()->value("Movies/MultiScrapeSaveEach", false).toBool();

    m_showMissingEpisodesHint = settings()->value("TvShows/ShowMissingEpisodesHint", true).toBool();

    m_extraFanartsMusicArtists = settings()->value("Music/Artists/ExtraFanarts", 0).toInt();

    PluginManager::instance()->loadSettings();
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

    // XBMC
    settings()->setValue("XBMC/RemoteHost", m_xbmcHost);
    settings()->setValue("XBMC/RemotePort", m_xbmcPort);
    settings()->setValue("XBMC/RemoteUser", m_xbmcUser);
    settings()->setValue("XBMC/RemotePassword", m_xbmcPassword);

    // Proxy
    settings()->setValue("Proxy/Enable", m_useProxy);
    settings()->setValue("Proxy/Type", m_proxyType);
    settings()->setValue("Proxy/Host", m_proxyHost);
    settings()->setValue("Proxy/Port", m_proxyPort);
    settings()->setValue("Proxy/Username", m_proxyUsername);
    settings()->setValue("Proxy/Password", m_proxyPassword);
    setupProxy();

    // Tv Shows
    settings()->setValue("TvShows/DvdOrder", m_tvShowDvdOrder);

    // Warnings
    settings()->setValue("Warnings/DontShowDeleteImageConfirm", m_dontShowDeleteImageConfirm);

    settings()->beginWriteArray("Directories/Movies");
    for (int i=0, n=m_movieDirectories.count() ; i<n ; ++i) {
        settings()->setArrayIndex(i);
        settings()->setValue("path", m_movieDirectories.at(i).path);
        settings()->setValue("sepFolders", m_movieDirectories.at(i).separateFolders);
        settings()->setValue("autoReload", m_movieDirectories.at(i).autoReload);
    }
    settings()->endArray();

    settings()->beginWriteArray("Directories/TvShows");
    for (int i=0, n=m_tvShowDirectories.count() ; i<n ; ++i) {
        settings()->setArrayIndex(i);
        settings()->setValue("path", m_tvShowDirectories.at(i).path);
        settings()->setValue("autoReload", m_tvShowDirectories.at(i).autoReload);
    }
    settings()->endArray();

    settings()->beginWriteArray("Directories/Concerts");
    for (int i=0, n=m_concertDirectories.count() ; i<n ; ++i) {
        settings()->setArrayIndex(i);
        settings()->setValue("path", m_concertDirectories.at(i).path);
        settings()->setValue("sepFolders", m_concertDirectories.at(i).separateFolders);
        settings()->setValue("autoReload", m_concertDirectories.at(i).autoReload);
    }
    settings()->endArray();

    settings()->beginWriteArray("Directories/Downloads");
    for (int i=0, n=m_downloadDirectories.count() ; i<n ; ++i) {
        settings()->setArrayIndex(i);
        settings()->setValue("path", m_downloadDirectories.at(i).path);
        settings()->setValue("sepFolders", m_downloadDirectories.at(i).separateFolders);
        settings()->setValue("autoReload", m_downloadDirectories.at(i).autoReload);
    }
    settings()->endArray();

    settings()->beginWriteArray("Directories/Music");
    for (int i=0, n=m_musicDirectories.count() ; i<n ; ++i) {
        settings()->setArrayIndex(i);
        settings()->setValue("path", m_musicDirectories.at(i).path);
        settings()->setValue("sepFolders", m_musicDirectories.at(i).separateFolders);
        settings()->setValue("autoReload", m_musicDirectories.at(i).autoReload);
    }
    settings()->endArray();

    settings()->setValue("excludeWords", m_excludeWords);

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings(*settings());
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings(*settings());
    }
    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings(*settings());
    }
    foreach (MusicScraperInterface *scraper, Manager::instance()->musicScrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings(*settings());
    }
    foreach (ImageProviderInterface *scraper, Manager::instance()->imageProviders()) {
        if (scraper->hasSettings())
            scraper->saveSettings(*settings());
    }

    settings()->setValue("Scraper/CurrentMovieScraper", m_currentMovieScraper);

    settings()->beginWriteArray("AllDataFiles");
    for (int i=0, n=m_dataFiles.count() ; i<n ; ++i) {
        settings()->setArrayIndex(i);
        settings()->setValue("type", m_dataFiles.at(i).type());
        settings()->setValue("fileName", m_dataFiles.at(i).fileName());
        settings()->setValue("pos", m_dataFiles.at(i).pos());
    }
    settings()->endArray();

    settings()->setValue("MovieSetArtwork/StoringType", m_movieSetArtworkType);
    settings()->setValue("MovieSetArtwork/Directory", m_movieSetArtworkDirectory);

    QList<QVariant> columns;
    foreach (const MediaStatusColumns &column, m_mediaStatusColumns)
        columns.append(column);
    settings()->setValue("MediaStatusColumns", columns);

    int i=0;
    settings()->beginWriteArray("CustomMovieScraper");
    QMapIterator<int, QString> it(m_customMovieScraper);
    while (it.hasNext()) {
        it.next();
        settings()->setArrayIndex(i++);
        settings()->setValue("Info", it.key());
        settings()->setValue("Scraper", it.value());
    }
    settings()->endArray();

    i=0;
    settings()->beginWriteArray("CustomTvScraper");
    QMapIterator<int, QString> itTv(m_customTvScraper);
    while (itTv.hasNext()) {
        itTv.next();
        settings()->setArrayIndex(i++);
        settings()->setValue("Info", itTv.key());
        settings()->setValue("Scraper", itTv.value());
    }
    settings()->endArray();

    settings()->setValue("Downloads/Unrar", m_unrar);
    settings()->setValue("Downloads/MakeMkvCon", m_makeMkvCon);
    settings()->setValue("Downloads/DeleteArchives", m_deleteArchives);
    settings()->setValue("Downloads/KeepSource", m_keepDownloadSource);

    settings()->setValue("TvShows/ShowMissingEpisodesHint", m_showMissingEpisodesHint);

    settings()->setValue("Movies/MultiScrapeOnlyWithId", m_multiScrapeOnlyWithId);
    settings()->setValue("Movies/MultiScrapeSaveEach", m_multiScrapeSaveEach);

    settings()->setValue("Music/Artists/ExtraFanarts", m_extraFanartsMusicArtists);

    PluginManager::instance()->saveSettings();

    settings()->sync();

    emit sigSettingsSaved();
}

/**
 * @brief Sets up the proxy
 */
void Settings::setupProxy()
{
    QNetworkProxy proxy;
    if (!m_useProxy)
        proxy.setType(QNetworkProxy::NoProxy);
    else if (m_proxyType == 0)
        proxy.setType(QNetworkProxy::HttpProxy);
    else
        proxy.setType(QNetworkProxy::Socks5Proxy);
    proxy.setHostName(m_proxyHost);
    proxy.setPort(m_proxyPort);
    proxy.setUser(m_proxyUsername);
    proxy.setPassword(m_proxyPassword);
    QNetworkProxy::setApplicationProxy(proxy);
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

/**
 * @brief Settings::mainWindowMaximized
 * @return
 */
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

/**
 * @brief Returns a list of movie directories
 * @return List of movie directories
 */
QList<SettingsDir> Settings::movieDirectories()
{
    return m_movieDirectories;
}

/**
 * @brief Returns a list of tv show directories
 * @return List of tv show directories
 */
QList<SettingsDir> Settings::tvShowDirectories()
{
    return m_tvShowDirectories;
}

/**
 * @brief Returns a list of concert directories
 * @return List of concert directories
 */
QList<SettingsDir> Settings::concertDirectories()
{
    return m_concertDirectories;
}

QList<SettingsDir> Settings::musicDirectories()
{
    return m_musicDirectories;
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

/**
 * @brief Holds if a proxy should be used
 * @return Proxy enabled
 */
bool Settings::useProxy()
{
    return m_useProxy;
}

/**
 * @brief Holds the type of the proxy (0 HTTP, 1 SOCKS5)
 * @return Proxy type
 */
int Settings::proxyType()
{
    return m_proxyType;
}

/**
 * @brief Holds the host of the proxy
 * @return Proxy host
 */
QString Settings::proxyHost()
{
    return m_proxyHost;
}

/**
 * @brief Holds the port of the proxy
 * @return Proxy port
 */
int Settings::proxyPort()
{
    return m_proxyPort;
}

/**
 * @brief Holds the username of the proxy
 * @return Proxy username
 */
QString Settings::proxyUsername()
{
    return m_proxyUsername;
}

/**
 * @brief Holds the password of the proxy
 * @return Proxy password
 */
QString Settings::proxyPassword()
{
    return m_proxyPassword;
}

/**
 * @brief Settings::autoLoadStreamDetails
 * @return
 */
bool Settings::autoLoadStreamDetails()
{
    return m_autoLoadStreamDetails;
}

/**
 * @brief Settings::dataFiles
 * @param type
 * @return
 */
QList<DataFile> Settings::dataFiles(int type)
{
    QList<DataFile> files;
    foreach (const DataFile &file, m_dataFiles) {
        if (file.type() == type)
            files.append(file);
    }
    qSort(files.begin(), files.end(), DataFile::lessThan);
    return files;
}

QList<DataFile> Settings::dataFilesFrodo(int type)
{
    if (type == -1)
        return m_initialDataFilesFrodo;

    QList<DataFile> files;
    foreach (const DataFile &file, m_initialDataFilesFrodo) {
        if (file.type() == type)
            files.append(file);
    }
    qSort(files.begin(), files.end(), DataFile::lessThan);
    return files;
}

/**
 * @brief Settings::usePlotForOutline
 * @return
 */
bool Settings::usePlotForOutline()
{
    return m_usePlotForOutline;
}

QString Settings::xbmcHost()
{
    return m_xbmcHost;
}

int Settings::xbmcPort()
{
    return m_xbmcPort;
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

/**
 * @brief Settings::setMainWindowMaximized
 * @param max
 */
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

/**
 * @brief Sets the movie directories
 * @param dirs Dirs to set
 */
void Settings::setMovieDirectories(QList<SettingsDir> dirs)
{
    m_movieDirectories = dirs;
}

/**
 * @brief Sets the tv show directories
 * @param dirs Dirs to set
 */
void Settings::setTvShowDirectories(QList<SettingsDir> dirs)
{
    m_tvShowDirectories = dirs;
}

/**
 * @brief Sets the concert directories
 * @param dirs Dirs to set
 */
void Settings::setConcertDirectories(QList<SettingsDir> dirs)
{
    m_concertDirectories = dirs;
}

void Settings::setMusicDirectories(QList<SettingsDir> dirs)
{
    m_musicDirectories = dirs;
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

/**
 * @brief Sets state of debug mode
 * @param enabled
 */
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
 * @param use
 */
void Settings::setUseYoutubePluginUrls(bool use)
{
    m_youtubePluginUrls = use;
}

/**
 * @brief Settings::setDataFiles
 * @param files
 */
void Settings::setDataFiles(QList<DataFile> files)
{
    m_dataFiles = files;
}

/**
 * @brief Sets if a proxy should be used
 * @param use Enable proxy
 */
void Settings::setUseProxy(bool use)
{
    m_useProxy = use;
}

/**
 * @brief Sets the proxy type
 * @param type 0 HTTP, 1 SOCKS5
 */
void Settings::setProxyType(int type)
{
    m_proxyType = type;
}

/**
 * @brief Sets the host of the proxy
 * @param host Proxy host
 */
void Settings::setProxyHost(QString host)
{
    m_proxyHost = host;
}

/**
 * @brief Sets the port of the proxy
 * @param port Proxy port
 */
void Settings::setProxyPort(int port)
{
    m_proxyPort = port;
}

/**
 * @brief Sets the username to use when connecting to the proxy
 * @param username Proxy username
 */
void Settings::setProxyUsername(QString username)
{
    m_proxyUsername = username;
}

/**
 * @brief Sets the password to use when connecting to the proxy
 * @param password Proxy password
 */
void Settings::setProxyPassword(QString password)
{
    m_proxyPassword = password;
}

/**
 * @brief Settings::setAutoLoadStreamDetails
 * @param autoLoad
 */
void Settings::setAutoLoadStreamDetails(bool autoLoad)
{
    m_autoLoadStreamDetails = autoLoad;
}

/**
 * @brief Settings::setUsePlotForOutline
 * @param use
 */
void Settings::setUsePlotForOutline(bool use)
{
    m_usePlotForOutline = use;
}

void Settings::setXbmcHost(QString host)
{
    m_xbmcHost = host;
}

void Settings::setXbmcPort(int port)
{
    m_xbmcPort = port;
}

QList<int> Settings::scraperInfos(MainWidgets widget, QString scraperId)
{
    QString item = "unknown";
    if (widget == WidgetMovies)
        item = "Movies";
    else if (widget == WidgetConcerts)
        item = "Concerts";
    else if (widget == WidgetTvShows)
        item = "TvShows";
    else if (widget == WidgetMusic)
        item = "Music";
    QList<int> infos;
    foreach (const QString &info, settings()->value(QString("Scrapers/%1/%2").arg(item).arg(scraperId)).toString().split(","))
        infos << info.toInt();

    if (!infos.isEmpty() && infos.first() == 0)
        infos.clear();

    return infos;
}

void Settings::setScraperInfos(MainWidgets widget, QString scraperNo, QList<int> items)
{
    QString item = "unknown";
    if (widget == WidgetMovies)
        item = "Movies";
    else if (widget == WidgetConcerts)
        item = "Concerts";
    else if (widget == WidgetTvShows)
        item = "TvShows";
    else if (widget == WidgetMusic)
        item = "Music";
    QStringList infos;
    foreach (int info, items)
        infos << QString("%1").arg(info);
    settings()->setValue(QString("Scrapers/%1/%2").arg(item).arg(scraperNo), infos.join(","));
}

bool Settings::downloadActorImages()
{
    return m_downloadActorImages;
}

void Settings::setDownloadActorImages(bool download)
{
    m_downloadActorImages = download;
}

void Settings::renamePatterns(int renameType, QString &fileNamePattern, QString &fileNamePatternMulti, QString &directoryPattern, QString &seasonPattern)
{
    QString fileNamePatternDefault = "<title>.<extension>";
    QString fileNamePatternMultiDefault = "<title>-part<partNo>.<extension>";
    if (renameType == Renamer::TypeTvShows) {
        fileNamePatternDefault = "S<season>E<episode> - <title>.<extension>";
        fileNamePatternMultiDefault = "S<season>E<episode> - <title>-part<partNo>.<extension>";
    }
    fileNamePattern = settings()->value(QString("RenamePattern/%1/FileName").arg(renameType), fileNamePatternDefault).toString();
    fileNamePatternMulti = settings()->value(QString("RenamePattern/%1/FileNameMulti").arg(renameType), fileNamePatternMultiDefault).toString();
    directoryPattern = settings()->value(QString("RenamePattern/%1/DirectoryPattern").arg(renameType), "<title> (<year>)").toString();
    seasonPattern = settings()->value(QString("RenamePattern/%1/SeasonPattern").arg(renameType), "Season <season>").toString();
}

void Settings::setRenamePatterns(int renameType, QString fileNamePattern, QString fileNamePatternMulti, QString directoryPattern, QString seasonPattern)
{
    settings()->setValue(QString("RenamePattern/%1/FileName").arg(renameType), fileNamePattern);
    settings()->setValue(QString("RenamePattern/%1/FileNameMulti").arg(renameType), fileNamePatternMulti);
    settings()->setValue(QString("RenamePattern/%1/DirectoryPattern").arg(renameType), directoryPattern);
    settings()->setValue(QString("RenamePattern/%1/SeasonPattern").arg(renameType), seasonPattern);
}

void Settings::setRenamings(int renameType, bool files, bool folders, bool seasonDirectories)
{
    settings()->setValue(QString("RenamePattern/%1/RenameFiles").arg(renameType), files);
    settings()->setValue(QString("RenamePattern/%1/RenameFolders").arg(renameType), folders);
    settings()->setValue(QString("RenamePattern/%1/UseSeasonDirectories").arg(renameType), seasonDirectories);
}

void Settings::renamings(int renameType, bool &files, bool &folders, bool &seasonDirectories)
{
    files = settings()->value(QString("RenamePattern/%1/RenameFiles").arg(renameType), true).toBool();
    folders = settings()->value(QString("RenamePattern/%1/RenameFolders").arg(renameType), true).toBool();
    seasonDirectories = settings()->value(QString("RenamePattern/%1/UseSeasonDirectories").arg(renameType), true).toBool();
}

int Settings::tvShowUpdateOption()
{
    return settings()->value("TvShowUpdateOption", 0).toInt();
}

void Settings::setTvShowUpdateOption(int option)
{
    settings()->setValue("TvShowUpdateOption", option);
}

AdvancedSettings *Settings::advanced()
{
    return m_advancedSettings;
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
    return static_cast<MovieSetArtworkType>(m_movieSetArtworkType);
}

void Settings::setMovieSetArtworkDirectory(QString dir)
{
    m_movieSetArtworkDirectory = dir;
}

QString Settings::movieSetArtworkDirectory() const
{
    return m_movieSetArtworkDirectory;
}

void Settings::setMediaStatusColumns(QList<MediaStatusColumns> columns)
{
    m_mediaStatusColumns = columns;
}

QList<MediaStatusColumns> Settings::mediaStatusColumns() const
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

QMap<int, QString> Settings::customMovieScraper() const
{
    return m_customMovieScraper;
}

void Settings::setCustomMovieScraper(QMap<int, QString> customMovieScraper)
{
    m_customMovieScraper = customMovieScraper;
}

QMap<int, QString> Settings::customTvScraper() const
{
    return m_customTvScraper;
}

void Settings::setCustomTvScraper(QMap<int, QString> customTvScraper)
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

void Settings::setDownloadDirectories(QList<SettingsDir> dirs)
{
    m_downloadDirectories = dirs;
}

QList<SettingsDir> Settings::downloadDirectories()
{
    return m_downloadDirectories;
}

void Settings::setUnrar(QString unrar)
{
    m_unrar = unrar;
}

QString Settings::unrar()
{
    return m_unrar;
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
    if (advanced()->portableMode())
        return applicationDir();
    else
        return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

QString Settings::imageCacheDir()
{
    if (advanced()->portableMode())
        return applicationDir();
    else
        return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

QString Settings::exportTemplatesDir()
{
    if (advanced()->portableMode())
        return applicationDir() + QDir::separator() + "export_themes";
    else
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

void Settings::setMakeMkvCon(QString makeMkvCon)
{
    m_makeMkvCon = makeMkvCon;
}

QString Settings::makeMkvCon()
{
    return m_makeMkvCon;
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

void Settings::setXbmcUser(QString user)
{
    m_xbmcUser = user;
}

QString Settings::xbmcUser()
{
    return m_xbmcUser;
}

void Settings::setXbmcPassword(QString password)
{
    m_xbmcPassword = password;
}

QString Settings::xbmcPassword()
{
    return m_xbmcPassword;
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

QStringList Settings::pluginDirs()
{
#if defined(PLUGIN_DIR_OVERRIDE)
    #define str_(x) #x
    #define str(x) str_(x)
    return QStringList() << str(PLUGIN_DIR_OVERRIDE);
#endif

#if defined(Q_OS_MAC)
    QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    if (dirs.isEmpty())
        return QStringList();
    QDir pluginDir(dirs.first());
    if (!pluginDir.cd("plugins") && !pluginDir.mkdir("plugins"))
        return QStringList();
    pluginDir.cd("plugins");
    return QStringList() << pluginDir.absolutePath();
#elif defined(Q_OS_WIN)
    QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    if (advanced()->portableMode())
        dirs = QStringList() << applicationDir();
    if (dirs.isEmpty())
        return QStringList();
    QDir pluginDir(dirs.first());
    if (!pluginDir.cd("plugins") && !pluginDir.mkdir("plugins"))
        return QStringList();
    pluginDir.cd("plugins");
    return QStringList() << pluginDir.absolutePath();
#elif defined(Q_OS_UNIX)
    QStringList dirs;
    QStringList sDirs = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    if (!sDirs.isEmpty()) {
        QDir pluginDir(sDirs.first());
        if (!pluginDir.cd("plugins"))
            pluginDir.mkdir("plugins");
        if (pluginDir.cd("plugins"))
            dirs << pluginDir.absolutePath();
    }
    dirs << "/usr/lib/MediaElch" << "/usr/local/lib/MediaElch";
    return dirs;
#endif
    return QStringList();
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
