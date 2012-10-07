#include "Settings.h"

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
    m_instance = this;

    // Setup XML filenames
    m_movieNfoFiles.append(new DataFile(0, "<movie filename>.nfo", "<moviefile>.nfo", 0, true));
    m_movieNfoFiles.append(new DataFile(1, "movie.nfo", "movie.nfo", 1, false));

    m_moviePosterFiles.append(new DataFile(2, "<movie filename>.tbn", "<moviefile>.tbn", 0, true));
    m_moviePosterFiles.append(new DataFile(3, "movie.jpg", "movie.jpg", 1, false));
    m_moviePosterFiles.append(new DataFile(4, "movie.tbn", "movie.tbn", 2, false));
    m_moviePosterFiles.append(new DataFile(5, "poster.jpg", "poster.jpg", 3, false));
    m_moviePosterFiles.append(new DataFile(6, "poster.tbn", "poster.tbn", 4, false));
    m_moviePosterFiles.append(new DataFile(7, "folder.jpg", "folder.jpg", 5, false));

    m_movieFanartFiles.append(new DataFile(8, "<movie filename>-fanart.jpg", "<moviefile>-fanart.jpg", 0, true));
    m_movieFanartFiles.append(new DataFile(9, "fanart.jpg", "fanart.jpg", 1, false));

    m_tvShowPosterFiles.append(new DataFile(10, "season-all.tbn", "season-all.tbn", 0, true));
    m_tvShowPosterFiles.append(new DataFile(11, "poster.jpg", "poster.jpg", 1, true));

    m_tvShowBannerFiles.append(new DataFile(12, "banner.jpg", "banner.jpg", 0, true));
    m_tvShowBannerFiles.append(new DataFile(13, "folder.jpg", "folder.jpg", 1, true));

    m_concertNfoFiles.append(new DataFile(0, "<movie filename>.nfo", "<moviefile>.nfo", 0, true));
    m_concertPosterFiles.append(new DataFile(1, "<movie filename>.tbn", "<moviefile>.tbn", 0, true));
    m_concertFanartFiles.append(new DataFile(2, "<movie filename>-fanart.jpg", "<moviefile>-fanart.jpg", 0, true));
}

/**
 * @brief Returns an instance of the settings
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
    m_mainSplitterState = m_settings.value("MainSplitterState").toByteArray();
    m_debugModeActivated = m_settings.value("DebugModeActivated", false).toBool();
    m_debugLogPath = m_settings.value("DebugLogPath").toString();
    m_useCache = m_settings.value("UseCache", true).toBool();

    // Movie Directories
    m_movieDirectories.clear();
    int moviesSize = m_settings.beginReadArray("Directories/Movies");
    for (int i=0 ; i<moviesSize ; ++i) {
        m_settings.setArrayIndex(i);
        SettingsDir dir;
        dir.path = QDir::toNativeSeparators(m_settings.value("path").toString());
        dir.mediaCenterPath = m_settings.value("mediaCenterPath").toString();
        dir.separateFolders = m_settings.value("sepFolders", false).toBool();
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
        dir.mediaCenterPath = m_settings.value("mediaCenterPath").toString();
        dir.separateFolders = m_settings.value("sepFolders", false).toBool();
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
        dir.mediaCenterPath = m_settings.value("mediaCenterPath").toString();
        dir.separateFolders = m_settings.value("sepFolders", false).toBool();
        m_concertDirectories.append(dir);
    }
    m_settings.endArray();

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
    m_mediaCenterInterface = m_settings.value("MediaCenterInterface", MediaCenterInterfaces::XbmcXml).toInt();
    m_xbmcMysqlHost        = m_settings.value("XbmcMysql/Host").toString();
    m_xbmcMysqlDatabase    = m_settings.value("XbmcMysql/Database").toString();
    m_xbmcMysqlUser        = m_settings.value("XbmcMysql/User").toString();
    m_xbmcMysqlPassword    = m_settings.value("XbmcMysql/Password").toString();
    m_xbmcSqliteDatabase   = m_settings.value("XbmcSqlite/Database").toString();
    m_xbmcThumbnailPath    = m_settings.value("XbmcThumbnailpath").toString();
    m_youtubePluginUrls    = m_settings.value("UseYoutubePluginURLs", false).toBool();

    // XBMC xml file
    QList<DataFile*> files;
    files << m_movieNfoFiles << m_moviePosterFiles << m_movieFanartFiles << m_tvShowPosterFiles << m_tvShowBannerFiles;
    int movieDataFileSize = m_settings.beginReadArray("DataFiles");
    for (int i=0 ; i<movieDataFileSize ; ++i) {
        m_settings.setArrayIndex(i);
        foreach (DataFile *file, files) {
            if (file->id() != m_settings.value("id").toInt())
                continue;
            file->setEnabled(m_settings.value("enabled").toBool());
            file->setPos(m_settings.value("pos").toInt());
        }
    }
    m_settings.endArray();

    qSort(m_movieNfoFiles.begin(), m_movieNfoFiles.end(), DataFile::lessThan);
    qSort(m_moviePosterFiles.begin(), m_moviePosterFiles.end(), DataFile::lessThan);
    qSort(m_movieFanartFiles.begin(), m_movieFanartFiles.end(), DataFile::lessThan);
    qSort(m_tvShowPosterFiles.begin(), m_tvShowPosterFiles.end(), DataFile::lessThan);
    qSort(m_tvShowBannerFiles.begin(), m_tvShowBannerFiles.end(), DataFile::lessThan);
}

/**
 * @brief Saves all settings
 */
void Settings::saveSettings()
{
    m_settings.setValue("DebugModeActivated", m_debugModeActivated);
    m_settings.setValue("DebugLogPath", m_debugLogPath);
    m_settings.setValue("UseCache", m_useCache);

    m_settings.setValue("XbmcMysql/Host", m_xbmcMysqlHost);
    m_settings.setValue("XbmcMysql/Database", m_xbmcMysqlDatabase);
    m_settings.setValue("XbmcMysql/User", m_xbmcMysqlUser);
    m_settings.setValue("XbmcMysql/Password", m_xbmcMysqlPassword);
    m_settings.setValue("XbmcSqlite/Database", m_xbmcSqliteDatabase);
    m_settings.setValue("MediaCenterInterface", m_mediaCenterInterface);
    m_settings.setValue("XbmcThumbnailpath", m_xbmcThumbnailPath);
    m_settings.setValue("UseYoutubePluginURLs", m_youtubePluginUrls);

    m_settings.beginWriteArray("Directories/Movies");
    for (int i=0, n=m_movieDirectories.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", m_movieDirectories.at(i).path);
        m_settings.setValue("mediaCenterPath", m_movieDirectories.at(i).mediaCenterPath);
        m_settings.setValue("sepFolders", m_movieDirectories.at(i).separateFolders);
    }
    m_settings.endArray();

    m_settings.beginWriteArray("Directories/TvShows");
    for (int i=0, n=m_tvShowDirectories.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", m_tvShowDirectories.at(i).path);
        m_settings.setValue("mediaCenterPath", m_tvShowDirectories.at(i).mediaCenterPath);
    }
    m_settings.endArray();

    m_settings.beginWriteArray("Directories/Concerts");
    for (int i=0, n=m_concertDirectories.count() ; i<n ; ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", m_concertDirectories.at(i).path);
        m_settings.setValue("mediaCenterPath", m_concertDirectories.at(i).mediaCenterPath);
        m_settings.setValue("sepFolders", m_concertDirectories.at(i).separateFolders);
    }
    m_settings.endArray();

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

    qSort(m_movieNfoFiles.begin(), m_movieNfoFiles.end(), DataFile::lessThan);
    qSort(m_moviePosterFiles.begin(), m_moviePosterFiles.end(), DataFile::lessThan);
    qSort(m_movieFanartFiles.begin(), m_movieFanartFiles.end(), DataFile::lessThan);
    qSort(m_tvShowPosterFiles.begin(), m_tvShowPosterFiles.end(), DataFile::lessThan);
    qSort(m_tvShowBannerFiles.begin(), m_tvShowBannerFiles.end(), DataFile::lessThan);

    QList<DataFile*> files;
    files << m_movieNfoFiles << m_moviePosterFiles << m_movieFanartFiles << m_tvShowPosterFiles << m_tvShowBannerFiles;
    m_settings.beginWriteArray("DataFiles");
    int i=0;
    foreach (DataFile *file, files) {
        m_settings.setArrayIndex(i++);
        m_settings.setValue("id", file->id());
        m_settings.setValue("pos", file->pos());
        m_settings.setValue("enabled", file->enabled());
    }
    m_settings.endArray();
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
 * @brief Returns the number of the chosen MediaCenterInterface
 * @return Number of the chosen MediaCenterInterface
 */
int Settings::mediaCenterInterface()
{
    return m_mediaCenterInterface;
}

/**
 * @brief Returns the host of the MySQL database
 * @return Host of the MySQL db
 */
QString Settings::xbmcMysqlHost()
{
    return m_xbmcMysqlHost;
}

/**
 * @brief Returns the name of the MySQL database
 * @return Name of the MySQL db
 */
QString Settings::xbmcMysqlDatabase()
{
    return m_xbmcMysqlDatabase;
}

/**
 * @brief Returns the user of the MySQL database
 * @return User of the MySQL db
 */
QString Settings::xbmcMysqlUser()
{
    return m_xbmcMysqlUser;
}

/**
 * @brief Returns the password of the MySQL database
 * @return Password of the MySQL db
 */
QString Settings::xbmcMysqlPassword()
{
    return m_xbmcMysqlPassword;
}

/**
 * @brief Returns the path to the SQLite database
 * @return Path to SQLite database
 */
QString Settings::xbmcSqliteDatabase()
{
    return m_xbmcSqliteDatabase;
}

/**
 * @brief Returns the path to the thumbnails
 * @return Path to thumbnails
 */
QString Settings::xbmcThumbnailPath()
{
    return m_xbmcThumbnailPath;
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
 * @brief Returns a list of enabled DataFiles for movie nfos.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::enabledMovieNfoFiles()
{
    QList<DataFile*> files;
    foreach (DataFile* file, m_movieNfoFiles) {
        if (file->enabled())
            files.append(file);
    }

    return files;
}

/**
 * @brief Returns a list of enabled DataFiles for movie posters.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::enabledMoviePosterFiles()
{
    QList<DataFile*> files;
    foreach (DataFile* file, m_moviePosterFiles) {
        if (file->enabled())
            files.append(file);
    }

    return files;
}

/**
 * @brief Returns a list of enabled DataFiles for movie fanarts.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::enabledMovieFanartFiles()
{
    QList<DataFile*> files;
    foreach (DataFile* file, m_movieFanartFiles) {
        if (file->enabled())
            files.append(file);
    }

    return files;
}

/**
 * @brief Returns a list of enabled DataFiles for tv show posters.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::enabledTvShowPosterFiles()
{
    QList<DataFile*> files;
    foreach (DataFile* file, m_tvShowPosterFiles) {
        if (file->enabled())
            files.append(file);
    }

    return files;
}

/**
 * @brief Returns a list of enabled DataFiles for tv show banners.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::enabledTvShowBannerFiles()
{
    QList<DataFile*> files;
    foreach (DataFile* file, m_tvShowBannerFiles) {
        if (file->enabled())
            files.append(file);
    }

    return files;
}

/**
 * @brief Returns a list of enabled DataFiles for concert nfos.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::enabledConcertNfoFiles()
{
    QList<DataFile*> files;
    foreach (DataFile* file, m_concertNfoFiles) {
        if (file->enabled())
            files.append(file);
    }

    return files;
}

/**
 * @brief Returns a list of enabled DataFiles for concert posters.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::enabledConcertPosterFiles()
{
    QList<DataFile*> files;
    foreach (DataFile* file, m_concertPosterFiles) {
        if (file->enabled())
            files.append(file);
    }

    return files;
}

/**
 * @brief Returns a list of enabled DataFiles for concert fanarts.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::enabledConcertFanartFiles()
{
    QList<DataFile*> files;
    foreach (DataFile* file, m_concertFanartFiles) {
        if (file->enabled())
            files.append(file);
    }

    return files;
}

/**
 * @brief Returns a list of all DataFiles for movie nfos.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::movieNfoFiles()
{
    return m_movieNfoFiles;
}

/**
 * @brief Returns a list of all DataFiles for movie posters.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::moviePosterFiles()
{
    return m_moviePosterFiles;
}

/**
 * @brief Returns a list of all DataFiles for movie fanarts.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::movieFanartFiles()
{
    return m_movieFanartFiles;
}

/**
 * @brief Returns a list of all DataFiles for tv show posters.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::tvShowPosterFiles()
{
    return m_tvShowPosterFiles;
}

/**
 * @brief Returns a list of all DataFiles for tv show banners.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::tvShowBannerFiles()
{
    return m_tvShowBannerFiles;
}

/**
 * @brief Returns a list of all DataFiles for concert nfos.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::concertNfoFiles()
{
    return m_concertNfoFiles;
}

/**
 * @brief Returns a list of all DataFiles for concert posters.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::concertPosterFiles()
{
    return m_concertPosterFiles;
}

/**
 * @brief Returns a list of all DataFiles for concert fanarts.
 * @return List of DataFile objects
 */
QList<DataFile*> Settings::concertFanartFiles()
{
    return m_concertFanartFiles;
}

/**
 * @brief Returns true if the cache should be used
 * @return Cache usage
 */
bool Settings::useCache()
{
    return m_useCache;
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
 * @brief Sets the path to the xbmc sqlite database
 * @param file Database file
 */
void Settings::setXbmcSqliteDatabase(QString file)
{
    m_xbmcSqliteDatabase = file;
}

/**
 * @brief Sets the path to the xbmc thumbnails
 * @param path Path to thumbnails
 */
void Settings::setXbmcThumbnailPath(QString path)
{
    m_xbmcThumbnailPath = path;
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
 * @brief Sets the mysql host
 * @param host
 */
void Settings::setXbmcMysqlHost(QString host)
{
    m_xbmcMysqlHost = host;
}

/**
 * @brief Sets the mysql database
 * @param db
 */
void Settings::setXbmcMysqlDatabase(QString db)
{
    m_xbmcMysqlDatabase = db;
}

/**
 * @brief Sets the mysql user
 * @param user
 */
void Settings::setXbmcMysqlUser(QString user)
{
    m_xbmcMysqlUser = user;
}

/**
 * @brief Sets the mysql password
 * @param password
 */
void Settings::setXbmcMysqlPassword(QString password)
{
    m_xbmcMysqlPassword = password;
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
 * @brief Sets the movie nfo files
 * @param files List of DataFile Objects
 */
void Settings::setMovieNfoFiles(QList<DataFile*> files)
{
    m_movieNfoFiles = files;
}

/**
 * @brief Sets the movie poster files
 * @param files List of DataFile Objects
 */
void Settings::setMoviePosterFiles(QList<DataFile*> files)
{
    m_moviePosterFiles = files;
}

/**
 * @brief Sets the movie fanart files
 * @param files List of DataFile Objects
 */
void Settings::setMovieFanartFiles(QList<DataFile*> files)
{
    m_movieFanartFiles = files;
}

/**
 * @brief Sets the tv show poster files
 * @param files List of DataFile Objects
 */
void Settings::setTvShowPosterFiles(QList<DataFile*> files)
{
    m_tvShowPosterFiles = files;
}

/**
 * @brief Sets the tv show banner files
 * @param files List of DataFile Objects
 */
void Settings::setTvShowBannerFiles(QList<DataFile*> files)
{
    m_tvShowBannerFiles = files;
}

/**
 * @brief Sets the concert nfo files
 * @param files List of DataFile Objects
 */
void Settings::setConcertNfoFiles(QList<DataFile*> files)
{
    m_concertNfoFiles = files;
}

/**
 * @brief Sets the concert poster files
 * @param files List of DataFile Objects
 */
void Settings::setConcertPosterFiles(QList<DataFile*> files)
{
    m_concertPosterFiles = files;
}

/**
 * @brief Sets the concert fanart files
 * @param files List of DataFile Objects
 */
void Settings::setConcertFanartFiles(QList<DataFile*> files)
{
    m_concertFanartFiles = files;
}

/**
 * @brief Sets the MediaCenterInterface
 * @param interface
 */
void Settings::setMediaCenterInterface(int interface)
{
    m_mediaCenterInterface = interface;
}

/**
 * @brief Sets if the cache should be used
 * @param useCache
 */
void Settings::setUseCache(bool useCache)
{
    m_useCache = useCache;
}
