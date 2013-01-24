#include "Settings.h"

#include <QNetworkProxy>
#include "data/ScraperInterface.h"
#include "globals/Manager.h"

Settings *Settings::m_instance = 0;

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings(QObject *parent) :
    QObject(parent)
{
    // Eden
    m_initialDataFilesEden.append(DataFile(DataFileType::MovieNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::MovieNfo, "movie.nfo", 1));
    m_initialDataFilesEden.append(DataFile(DataFileType::MoviePoster, "<baseFileName>.tbn", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::MovieBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::MovieCdArt, "cdart.png", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::MovieClearArt, "clearart.png", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::MovieLogo, "logo.png", 0));

    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowNfo, "tvshow.nfo", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowBackdrop, "fanart.jpg", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowBanner, "banner.jpg", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowCharacterArt, "character.png", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowClearArt, "clearart.png", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowLogo, "logo.png", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowPoster, "season-all.tbn", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowPoster, "poster.jpg", 1));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowSeasonPoster, "season<seasonNumber>.tbn", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowSeasonBackdrop, "season<seasonNumber>-fanart.tbn", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowSeasonBanner, "season<seasonNumber>-banner.tbn", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowEpisodeNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::TvShowEpisodeThumb, "<baseFileName>.tbn", 0));

    m_initialDataFilesEden.append(DataFile(DataFileType::ConcertNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::ConcertPoster, "<baseFileName>.tbn", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::ConcertBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::ConcertCdArt, "cdart.png", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::ConcertClearArt, "clearart.png", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::ConcertLogo, "logo.png", 0));

    // Frodo
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieNfo, "movie.nfo", 1));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MoviePoster, "<baseFileName>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieCdArt, "cdart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieClearArt, "clearart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieLogo, "logo.png", 0));

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

    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertPoster, "<baseFileName>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertCdArt, "cdart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertClearArt, "clearart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertLogo, "logo.png", 0));
}

/**
 * @brief Returns an instance of the settings
 * @param parent Parent widget
 * @return Instance of Settings
 */
Settings *Settings::instance(QObject *parent)
{
    if (m_instance == 0) {
        m_instance = new Settings(parent);
    }
    return m_instance;
}

/**
 * @brief Loads all settings
 */
void Settings::loadSettings()
{
    // Globals
    m_mainWindowSize = m_settings.value("MainWindowSize").toSize();
    m_mainWindowPosition = m_settings.value("MainWindowPosition").toPoint();
    m_mainWindowMaximized = m_settings.value("MainWindowMaximized").toBool();
    m_mainSplitterState = m_settings.value("MainSplitterState").toByteArray();
    m_debugModeActivated = m_settings.value("DebugModeActivated", false).toBool();
    m_debugLogPath = m_settings.value("DebugLogPath").toString();
    m_autoLoadStreamDetails = m_settings.value("AutoLoadStreamDetails", true).toBool();
    m_usePlotForOutline = m_settings.value("Movies/UsePlotForOutline", true).toBool();
    m_downloadActorImages = m_settings.value("DownloadActorImages", true).toBool();

    // XBMC
    m_xbmcHost = m_settings.value("XBMC/Host").toString();
    m_xbmcPort = m_settings.value("XBMC/Port").toInt();
    m_xbmcUsername = m_settings.value("XBMC/Username").toString();
    m_xbmcPassword = m_settings.value("XBMC/Password").toString();

    // Proxy
    m_useProxy = m_settings.value("Proxy/Enable", false).toBool();
    m_proxyType = m_settings.value("Proxy/Type", 0).toInt();
    m_proxyHost = m_settings.value("Proxy/Host").toString();
    m_proxyPort = m_settings.value("Proxy/Port", 0).toInt();
    m_proxyUsername = m_settings.value("Proxy/Username").toString();
    m_proxyPassword = m_settings.value("Proxy/Password").toString();
    setupProxy();

    // Movie Directories
    m_movieDirectories.clear();
    int moviesSize = m_settings.beginReadArray("Directories/Movies");
    for (int i=0 ; i<moviesSize ; ++i) {
        m_settings.setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(m_settings.value("path").toString());
        dir.separateFolders = m_settings.value("sepFolders", false).toBool();
        dir.autoReload = m_settings.value("autoReload", false).toBool();
        m_movieDirectories.append(dir);
    }
    m_settings.endArray();

    // TV Show Directories
    m_tvShowDirectories.clear();
    int tvShowSize = m_settings.beginReadArray("Directories/TvShows");
    for (int i=0 ; i<tvShowSize ; ++i) {
        m_settings.setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(m_settings.value("path").toString());
        dir.separateFolders = m_settings.value("sepFolders", false).toBool();
        dir.autoReload = m_settings.value("autoReload", false).toBool();
        m_tvShowDirectories.append(dir);
    }
    m_settings.endArray();

    // Concert Directories
    m_concertDirectories.clear();
    int concertsSize = m_settings.beginReadArray("Directories/Concerts");
    for (int i=0 ; i<concertsSize ; ++i) {
        m_settings.setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(m_settings.value("path").toString());
        dir.separateFolders = m_settings.value("sepFolders", false).toBool();
        dir.autoReload = m_settings.value("autoReload", false).toBool();
        m_concertDirectories.append(dir);
    }
    m_settings.endArray();

    m_excludeWords = m_settings.value("excludeWords").toString();

    // Scrapers
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings())
            scraper->loadSettings();
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings())
            scraper->loadSettings();
    }
    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings())
            scraper->loadSettings();
    }

    // Media Centers
    m_youtubePluginUrls    = m_settings.value("UseYoutubePluginURLs", false).toBool();

    // Data Files
    QList<DataFile> dataFiles;
    int dataFileSize = m_settings.beginReadArray("AllDataFiles");
    for (int i=0 ; i<dataFileSize ; ++i) {
        m_settings.setArrayIndex(i);
        int type = m_settings.value("type").toInt();
        QString fileName = m_settings.value("fileName").toString();
        int pos = m_settings.value("pos").toInt();
        DataFile f(type, fileName, pos);
        dataFiles.append(f);
    }
    m_settings.endArray();
    if (dataFiles.isEmpty())
        m_dataFiles = m_initialDataFilesFrodo;
    else
        m_dataFiles = dataFiles;
}

/**
 * @brief Saves all settings
 */
void Settings::saveSettings()
{
    m_settings.setValue("DebugModeActivated", m_debugModeActivated);
    m_settings.setValue("DebugLogPath", m_debugLogPath);
    m_settings.setValue("AutoLoadStreamDetails", m_autoLoadStreamDetails);

    m_settings.setValue("UseYoutubePluginURLs", m_youtubePluginUrls);
    m_settings.setValue("Movies/UsePlotForOutline", m_usePlotForOutline);
    m_settings.setValue("DownloadActorImages", m_downloadActorImages);

    // XBMC
    m_settings.setValue("XBMC/Host", m_xbmcHost);
    m_settings.setValue("XBMC/Port", m_xbmcPort);
    m_settings.setValue("XBMC/Username", m_xbmcUsername);
    m_settings.setValue("XBMC/Password", m_xbmcPassword);

    // Proxy
    m_settings.setValue("Proxy/Enable", m_useProxy);
    m_settings.setValue("Proxy/Type", m_proxyType);
    m_settings.setValue("Proxy/Host", m_proxyHost);
    m_settings.setValue("Proxy/Port", m_proxyPort);
    m_settings.setValue("Proxy/Username", m_proxyUsername);
    m_settings.setValue("Proxy/Password", m_proxyPassword);
    setupProxy();

    m_settings.beginWriteArray("Directories/Movies");
    for (int i=0, n=m_movieDirectories.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", m_movieDirectories.at(i).path);
        m_settings.setValue("sepFolders", m_movieDirectories.at(i).separateFolders);
        m_settings.setValue("autoReload", m_movieDirectories.at(i).autoReload);
    }
    m_settings.endArray();

    m_settings.beginWriteArray("Directories/TvShows");
    for (int i=0, n=m_tvShowDirectories.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", m_tvShowDirectories.at(i).path);
        m_settings.setValue("autoReload", m_tvShowDirectories.at(i).autoReload);
    }
    m_settings.endArray();

    m_settings.beginWriteArray("Directories/Concerts");
    for (int i=0, n=m_concertDirectories.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", m_concertDirectories.at(i).path);
        m_settings.setValue("sepFolders", m_concertDirectories.at(i).separateFolders);
        m_settings.setValue("autoReload", m_concertDirectories.at(i).autoReload);
    }
    m_settings.endArray();

    m_settings.setValue("excludeWords", m_excludeWords);

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings();
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings();
    }
    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings();
    }

    m_settings.beginWriteArray("AllDataFiles");
    for (int i=0, n=m_dataFiles.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("type", m_dataFiles.at(i).type());
        m_settings.setValue("fileName", m_dataFiles.at(i).fileName());
        m_settings.setValue("pos", m_dataFiles.at(i).pos());
    }
    m_settings.endArray();
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

QString Settings::xbmcUsername()
{
    return m_xbmcUsername;
}

QString Settings::xbmcPassword()
{
    return m_xbmcPassword;
}

/*** SETTER ***/

/**
 * @brief Sets the size of the main window
 * @param mainWindowSize Size of the main window
 */
void Settings::setMainWindowSize(QSize mainWindowSize)
{
    m_mainWindowSize = mainWindowSize;
    m_settings.setValue("MainWindowSize", mainWindowSize);
}

/**
 * @brief Sets the position of the main window
 * @param mainWindowPosition Position of the main window
 */
void Settings::setMainWindowPosition(QPoint mainWindowPosition)
{
    m_mainWindowPosition = mainWindowPosition;
    m_settings.setValue("MainWindowPosition", mainWindowPosition);
}

/**
 * @brief Settings::setMainWindowMaximized
 * @param max
 */
void Settings::setMainWindowMaximized(bool max)
{
    m_mainWindowMaximized = max;
    m_settings.setValue("MainWindowMaximized", max);
}

/**
 * @brief Sets the state of the main splitter
 * @param state State of the splitter
 */
void Settings::setMainSplitterState(QByteArray state)
{
    m_mainSplitterState = state;
    m_settings.setValue("MainSplitterState", state);
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

void Settings::setXbmcUsername(QString username)
{
    m_xbmcUsername = username;
}

void Settings::setXbmcPassword(QString password)
{
    m_xbmcPassword = password;
}

QList<int> Settings::scraperInfos(MainWidgets widget, int scraperNo)
{
    QString item = "unknown";
    if (widget == WidgetMovies)
        item = "Movies";
    else if (widget == WidgetConcerts)
        item = "Concerts";
    else if (widget == WidgetTvShows)
        item = "TvShows";
    return m_settings.value(QString("Scrapers/%1/%2").arg(item).arg(scraperNo)).value<QList<int> >();
}

void Settings::setScraperInfos(MainWidgets widget, int scraperNo, QList<int> items)
{
    QString item = "unknown";
    if (widget == WidgetMovies)
        item = "Movies";
    else if (widget == WidgetConcerts)
        item = "Concerts";
    else if (widget == WidgetTvShows)
        item = "TvShows";
    m_settings.setValue(QString("Scrapers/%1/%2").arg(item).arg(scraperNo), QVariant::fromValue<QList<int> >(items));
}

bool Settings::downloadActorImages()
{
    return m_downloadActorImages;
}

void Settings::setDownloadActorImages(bool download)
{
    m_downloadActorImages = download;
}

void Settings::loadEdenDefaults()
{
    m_dataFiles = m_initialDataFilesEden;
}

void Settings::loadFrodoDefaults()
{
    m_dataFiles = m_initialDataFilesFrodo;
}
