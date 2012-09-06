#include "Manager.h"

#include <QApplication>
#include "mediaCenterPlugins/XbmcSql.h"
#include "mediaCenterPlugins/XbmcXml.h"
#include "scrapers/Cinefacts.h"
#include "scrapers/OFDb.h"
#include "scrapers/TheTvDb.h"
#include "scrapers/TMDb.h"
#include "scrapers/VideoBuster.h"

/**
 * @brief Manager::Manager
 * @param parent
 */
Manager::Manager(QObject *parent) :
    QObject(parent)
{
    m_scrapers.append(new TMDb(this));
    m_scrapers.append(new Cinefacts(this));
    m_scrapers.append(new OFDb(this));
    m_scrapers.append(new VideoBuster(this));
    m_tvScrapers.append(new TheTvDb(this));
    m_movieFileSearcher = new MovieFileSearcher(this);
    m_tvShowFileSearcher = new TvShowFileSearcher(this);
    m_movieModel = new MovieModel(this);
    m_tvShowModel = new TvShowModel(this);
    m_tvShowProxyModel = new TvShowProxyModel(this);

    m_mediaCenters.append(new XbmcXml(this));
    m_mediaCenters.append(new XbmcSql(this, "xbmc"));
    m_mediaCenters.append(new XbmcSql(this, "xbmc"));

    m_mediaCentersTvShow.append(new XbmcXml(this));
    m_mediaCentersTvShow.append(new XbmcSql(this, "xbmcTvShow"));
    m_mediaCentersTvShow.append(new XbmcSql(this, "xbmcTvShow"));
}

/**
 * @brief Manager::~Manager
 */
Manager::~Manager()
{
}

/**
 * @brief Returns an instance of the Manager
 * @return Instance of Manager
 */
Manager* Manager::instance()
{
    static Manager *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new Manager(qApp);
    }
    return m_instance;
}

/**
 * @brief Sets up the MediaCenterInterfaces based on the settings
 */
void Manager::setupMediaCenterInterface()
{
    if (SettingsWidget::instance()->mediaCenterInterface() == MediaCenterInterfaces::XbmcMysql) {
        MediaCenterInterface *interface = m_mediaCenters.at(1);
        static_cast<XbmcSql*>(interface)->connectMysql(SettingsWidget::instance()->xbmcMysqlHost(), SettingsWidget::instance()->xbmcMysqlDatabase(),
                                                       SettingsWidget::instance()->xbmcMysqlUser(), SettingsWidget::instance()->xbmcMysqlPassword());
        interface = m_mediaCentersTvShow.at(1);
        static_cast<XbmcSql*>(interface)->connectMysql(SettingsWidget::instance()->xbmcMysqlHost(), SettingsWidget::instance()->xbmcMysqlDatabase(),
                                                       SettingsWidget::instance()->xbmcMysqlUser(), SettingsWidget::instance()->xbmcMysqlPassword());
    } else if (SettingsWidget::instance()->mediaCenterInterface() == MediaCenterInterfaces::XbmcSqlite) {
        MediaCenterInterface *interface = m_mediaCenters.at(2);
        static_cast<XbmcSql*>(interface)->connectSqlite(SettingsWidget::instance()->xbmcSqliteDatabase());

        interface = m_mediaCentersTvShow.at(2);
        static_cast<XbmcSql*>(interface)->connectSqlite(SettingsWidget::instance()->xbmcSqliteDatabase());
    }
}

/**
 * @brief Calls the shutdown function in every MediaCenterInterface
 */
void Manager::shutdownMediaCenterInterfaces()
{
    for (int i=0, n=m_mediaCenters.count() ; i<n ; ++i)
        m_mediaCenters.at(i)->shutdown();
}

/**
 * @brief Returns the active MediaCenterInterface
 * @return Instance of a MediaCenterInterface
 */
MediaCenterInterface *Manager::mediaCenterInterface()
{
    if (SettingsWidget::instance()->mediaCenterInterface() == MediaCenterInterfaces::XbmcXml)
        return m_mediaCenters.at(0);
    else if (SettingsWidget::instance()->mediaCenterInterface() == MediaCenterInterfaces::XbmcMysql)
        return m_mediaCenters.at(1);
    else if (SettingsWidget::instance()->mediaCenterInterface() == MediaCenterInterfaces::XbmcSqlite)
        return m_mediaCenters.at(2);

    return m_mediaCenters.at(0);
}

MediaCenterInterface *Manager::mediaCenterInterfaceTvShow()
{
    if (SettingsWidget::instance()->mediaCenterInterface() == MediaCenterInterfaces::XbmcXml)
        return m_mediaCentersTvShow.at(0);
    else if (SettingsWidget::instance()->mediaCenterInterface() == MediaCenterInterfaces::XbmcMysql)
        return m_mediaCentersTvShow.at(1);
    else if (SettingsWidget::instance()->mediaCenterInterface() == MediaCenterInterfaces::XbmcSqlite)
        return m_mediaCentersTvShow.at(2);

    return m_mediaCentersTvShow.at(0);
}

/**
 * @brief Returns an instance of the movie file searcher
 * @return Instance of movie searcher
 */
MovieFileSearcher *Manager::movieFileSearcher()
{
    return m_movieFileSearcher;
}

/**
 * @brief Returns an instance of the tv show file searcher
 * @return Instance of tv show file searcher
 */
TvShowFileSearcher *Manager::tvShowFileSearcher()
{
    return m_tvShowFileSearcher;
}

/**
 * @brief Returns a list of all movie scrapers
 * @return List of pointers of movie scrapers
 */
QList<ScraperInterface*> Manager::scrapers()
{
    return m_scrapers;
}

/**
 * @brief Returns a list of all tv scrapers
 * @return List of pointers of tv scrapers
 */
QList<TvScraperInterface*> Manager::tvScrapers()
{
    return m_tvScrapers;
}

/**
 * @brief Returns an instance of the MovieModel
 * @return Instance of the MovieModel
 */
MovieModel *Manager::movieModel()
{
    return m_movieModel;
}

/**
 * @brief Returns an instance of the TvShowModel
 * @return Instance of the TvShowModel
 */
TvShowModel *Manager::tvShowModel()
{
    return m_tvShowModel;
}

/**
 * @brief Returns an instance of the TvShowProxyModel
 * @return Instance of the TvShowProxyModel
 */
TvShowProxyModel *Manager::tvShowProxyModel()
{
    return m_tvShowProxyModel;
}
