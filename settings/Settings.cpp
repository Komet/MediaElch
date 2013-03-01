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
    m_advancedSettings = new AdvancedSettings(parent);

    // Eden
    m_initialDataFilesEden.append(DataFile(DataFileType::MovieNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::MoviePoster, "<baseFileName>.tbn", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::MovieBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::MovieCdArt, "disc.png", 0));
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
    m_initialDataFilesEden.append(DataFile(DataFileType::ConcertCdArt, "disc.png", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::ConcertClearArt, "clearart.png", 0));
    m_initialDataFilesEden.append(DataFile(DataFileType::ConcertLogo, "logo.png", 0));

    // Frodo
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieNfo, "<baseFileName>.nfo", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MoviePoster, "<baseFileName>-poster.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieBackdrop, "<baseFileName>-fanart.jpg", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::MovieCdArt, "disc.png", 0));
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
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertCdArt, "disc.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertClearArt, "clearart.png", 0));
    m_initialDataFilesFrodo.append(DataFile(DataFileType::ConcertLogo, "logo.png", 0));

    loadSettings();
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

void Settings::loadSettings()
{
    // Load old settings
    bool firstTime = m_settings.value("FirstTimeStartup", true).toBool();
    if (firstTime) {
        m_settings.setValue("FirstTimeStartup", false);
        QSettings settings("Daniel Kabel", "MediaElch");
        loadSettings(settings);
        saveSettings();
        return;
    }
    loadSettings(m_settings);
}
/**
 * @brief Loads all settings
 */
void Settings::loadSettings(QSettings &settings)
{
    // Globals
    m_mainWindowSize = settings.value("MainWindowSize").toSize();
    m_mainWindowPosition = settings.value("MainWindowPosition").toPoint();
    m_mainWindowMaximized = settings.value("MainWindowMaximized").toBool();
    m_mainSplitterState = settings.value("MainSplitterState").toByteArray();
    m_debugModeActivated = settings.value("DebugModeActivated", false).toBool();
    m_debugLogPath = settings.value("DebugLogPath").toString();
    m_autoLoadStreamDetails = settings.value("AutoLoadStreamDetails", true).toBool();
    m_usePlotForOutline = settings.value("Movies/UsePlotForOutline", true).toBool();
    m_downloadActorImages = settings.value("DownloadActorImages", true).toBool();
    m_ignoreArticlesWhenSorting = settings.value("IgnoreArticlesWhenSorting", false).toBool();

    // XBMC
    m_xbmcHost = settings.value("XBMC/RemoteHost").toString();
    m_xbmcPort = settings.value("XBMC/RemotePort", 9090).toInt();

    // Proxy
    m_useProxy = settings.value("Proxy/Enable", false).toBool();
    m_proxyType = settings.value("Proxy/Type", 0).toInt();
    m_proxyHost = settings.value("Proxy/Host").toString();
    m_proxyPort = settings.value("Proxy/Port", 0).toInt();
    m_proxyUsername = settings.value("Proxy/Username").toString();
    m_proxyPassword = settings.value("Proxy/Password").toString();
    setupProxy();

    // Movie Directories
    m_movieDirectories.clear();
    int moviesSize = settings.beginReadArray("Directories/Movies");
    for (int i=0 ; i<moviesSize ; ++i) {
        settings.setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(settings.value("path").toString());
        dir.separateFolders = settings.value("sepFolders", false).toBool();
        dir.autoReload = settings.value("autoReload", false).toBool();
        m_movieDirectories.append(dir);
    }
    settings.endArray();

    // TV Show Directories
    m_tvShowDirectories.clear();
    int tvShowSize = settings.beginReadArray("Directories/TvShows");
    for (int i=0 ; i<tvShowSize ; ++i) {
        settings.setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(settings.value("path").toString());
        dir.separateFolders = settings.value("sepFolders", false).toBool();
        dir.autoReload = settings.value("autoReload", false).toBool();
        m_tvShowDirectories.append(dir);
    }
    settings.endArray();

    // Concert Directories
    m_concertDirectories.clear();
    int concertsSize = settings.beginReadArray("Directories/Concerts");
    for (int i=0 ; i<concertsSize ; ++i) {
        settings.setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(settings.value("path").toString());
        dir.separateFolders = settings.value("sepFolders", false).toBool();
        dir.autoReload = settings.value("autoReload", false).toBool();
        m_concertDirectories.append(dir);
    }
    settings.endArray();

    m_excludeWords = settings.value("excludeWords").toString();
    if (m_excludeWords.isEmpty())
        m_excludeWords = "ac3,dts,custom,dc,divx,divx5,dsr,dsrip,dutch,dvd,dvdrip,dvdscr,dvdscreener,screener,dvdivx,cam,fragment,fs,hdtv,hdrip,hdtvrip,internal,limited,"
                         "multisubs,ntsc,ogg,ogm,pal,pdtv,proper,repack,rerip,retail,r3,r5,bd5,se,svcd,swedish,german,read.nfo,nfofix,unrated,ws,telesync,ts,telecine,tc,"
                         "brrip,bdrip,480p,480i,576p,576i,720p,720i,1080p,1080i,hrhd,hrhdtv,hddvd,bluray,x264,h264,xvid,xvidvd,xxx,www";

    // Scrapers
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings())
            scraper->loadSettings(settings);
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings())
            scraper->loadSettings(settings);
    }
    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings())
            scraper->loadSettings(settings);
    }

    // Media Centers
    m_youtubePluginUrls    = settings.value("UseYoutubePluginURLs", false).toBool();

    // Data Files
    QList<DataFile> dataFiles;
    int dataFileSize = settings.beginReadArray("AllDataFiles");
    for (int i=0 ; i<dataFileSize ; ++i) {
        settings.setArrayIndex(i);
        int type = settings.value("type").toInt();
        QString fileName = settings.value("fileName").toString();
        int pos = settings.value("pos").toInt();
        DataFile f(type, fileName, pos);
        dataFiles.append(f);
    }
    settings.endArray();
    if (dataFiles.isEmpty())
        m_dataFiles = m_initialDataFilesFrodo;
    else
        m_dataFiles = dataFiles;

    // Movie set artwork
    m_movieSetArtworkType = settings.value("MovieSetArtwork/Type", 0).toInt();
    m_movieSetArtworkDirectory = settings.value("MovieSetArtwork/Directory").toString();
    m_movieSetPosterFileName = settings.value("MovieSetArtwork/PosterFileName", "folder.jpg").toString();
    m_movieSetFanartFileName = settings.value("MovieSetArtwork/FanartFileName", "fanart.jpg").toString();

    // Media Status Columns
    m_mediaStatusColumns.clear();
    foreach (const QVariant &column, settings.value("MediaStatusColumns").toList())
        m_mediaStatusColumns.append(static_cast<MediaStatusColumns>(column.toInt()));

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
    m_settings.setValue("IgnoreArticlesWhenSorting", m_ignoreArticlesWhenSorting);

    // XBMC
    m_settings.setValue("XBMC/RemoteHost", m_xbmcHost);
    m_settings.setValue("XBMC/RemotePort", m_xbmcPort);

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
            scraper->saveSettings(m_settings);
    }
    foreach (TvScraperInterface *scraper, Manager::instance()->tvScrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings(m_settings);
    }
    foreach (ConcertScraperInterface *scraper, Manager::instance()->concertScrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings(m_settings);
    }

    m_settings.beginWriteArray("AllDataFiles");
    for (int i=0, n=m_dataFiles.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("type", m_dataFiles.at(i).type());
        m_settings.setValue("fileName", m_dataFiles.at(i).fileName());
        m_settings.setValue("pos", m_dataFiles.at(i).pos());
    }
    m_settings.endArray();

    m_settings.setValue("MovieSetArtwork/Type", m_movieSetArtworkType);
    m_settings.setValue("MovieSetArtwork/Directory", m_movieSetArtworkDirectory);
    m_settings.setValue("MovieSetArtwork/PosterFileName", m_movieSetPosterFileName);
    m_settings.setValue("MovieSetArtwork/FanartFileName", m_movieSetFanartFileName);

    QList<QVariant> columns;
    foreach (const MediaStatusColumns &column, m_mediaStatusColumns)
        columns.append(column);
    m_settings.setValue("MediaStatusColumns", columns);
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

void Settings::renamePatterns(int renameType, QString &fileNamePattern, QString &fileNamePatternMulti, QString &directoryPattern, QString &seasonPattern)
{
    fileNamePattern = m_settings.value(QString("RenamePattern/%1/FileName").arg(renameType), "<title>.<extension>").toString();
    fileNamePatternMulti = m_settings.value(QString("RenamePattern/%1/FileNameMulti").arg(renameType), "<title>-part<partNo>.<extension>").toString();
    directoryPattern = m_settings.value(QString("RenamePattern/%1/DirectoryPattern").arg(renameType), "<title> (<year>)").toString();
    seasonPattern = m_settings.value(QString("RenamePattern/%1/SeasonPattern").arg(renameType), "Season <season>").toString();
}

void Settings::setRenamePatterns(int renameType, QString fileNamePattern, QString fileNamePatternMulti, QString directoryPattern, QString seasonPattern)
{
    m_settings.setValue(QString("RenamePattern/%1/FileName").arg(renameType), fileNamePattern);
    m_settings.setValue(QString("RenamePattern/%1/FileNameMulti").arg(renameType), fileNamePatternMulti);
    m_settings.setValue(QString("RenamePattern/%1/DirectoryPattern").arg(renameType), directoryPattern);
    m_settings.setValue(QString("RenamePattern/%1/SeasonPattern").arg(renameType), seasonPattern);
}

void Settings::setRenamings(int renameType, bool files, bool folders, bool seasonDirectories)
{
    m_settings.setValue(QString("RenamePattern/%1/RenameFiles").arg(renameType), files);
    m_settings.setValue(QString("RenamePattern/%1/RenameFolders").arg(renameType), folders);
    m_settings.setValue(QString("RenamePattern/%1/UseSeasonDirectories").arg(renameType), seasonDirectories);
}

void Settings::renamings(int renameType, bool &files, bool &folders, bool &seasonDirectories)
{
    files = m_settings.value(QString("RenamePattern/%1/RenameFiles").arg(renameType), true).toBool();
    folders = m_settings.value(QString("RenamePattern/%1/RenameFolders").arg(renameType), true).toBool();
    seasonDirectories = m_settings.value(QString("RenamePattern/%1/UseSeasonDirectories").arg(renameType), true).toBool();
}

int Settings::tvShowUpdateOption()
{
    return m_settings.value("TvShowUpdateOption", 0).toInt();
}

void Settings::setTvShowUpdateOption(int option)
{
    m_settings.setValue("TvShowUpdateOption", option);
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

void Settings::setMovieSetPosterFileName(QString fileName)
{
    m_movieSetPosterFileName = fileName;
}

QString Settings::movieSetPosterFileName() const
{
    return m_movieSetPosterFileName;
}

void Settings::setMovieSetFanartFileName(QString fileName)
{
    m_movieSetFanartFileName = fileName;
}

QString Settings::movieSetFanartFileName() const
{
    return m_movieSetFanartFileName;
}

void Settings::setMediaStatusColumns(QList<MediaStatusColumns> columns)
{
    m_mediaStatusColumns = columns;
}

QList<MediaStatusColumns> Settings::mediaStatusColumns() const
{
    return m_mediaStatusColumns;
}
