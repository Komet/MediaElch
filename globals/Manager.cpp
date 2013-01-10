#include "globals/Manager.h"

#include <QApplication>
#include <QDesktopServices>
#include <QSqlQuery>
#include "globals/Globals.h"
#include "imageProviders/FanartTv.h"
#include "imageProviders/TMDbImages.h"
#include "imageProviders/TheTvDbImages.h"
#include "mediaCenterPlugins/XbmcXml.h"
#include "scrapers/Cinefacts.h"
#include "scrapers/IMDB.h"
#include "scrapers/OFDb.h"
#include "scrapers/TheTvDb.h"
#include "scrapers/TMDb.h"
#include "scrapers/TMDbConcerts.h"
#include "scrapers/VideoBuster.h"
#include "trailerProviders/HdTrailers.h"
#include "trailerProviders/MovieMaze.h"

/**
 * @brief Manager::Manager
 * @param parent
 */
Manager::Manager(QObject *parent) :
    QObject(parent)
{
    m_scrapers.append(new TMDb(this));
    m_scrapers.append(new IMDB(this));
    m_scrapers.append(new Cinefacts(this));
    m_scrapers.append(new OFDb(this));
    m_scrapers.append(new VideoBuster(this));
    m_tvScrapers.append(new TheTvDb(this));
    m_concertScrapers.append(new TMDbConcerts(this));
    m_movieFileSearcher = new MovieFileSearcher(this);
    m_tvShowFileSearcher = new TvShowFileSearcher(this);
    m_concertFileSearcher = new ConcertFileSearcher(this);
    m_movieModel = new MovieModel(this);
    m_tvShowModel = new TvShowModel(this);
    m_tvShowProxyModel = new TvShowProxyModel(this);
    m_concertModel = new ConcertModel(this);
    m_database = new Database(this);

    m_mediaCenters.append(new XbmcXml(this));
    m_mediaCentersTvShow.append(new XbmcXml(this));
    m_mediaCentersConcert.append(new XbmcXml(this));

    m_imageProviders.append(new FanartTv(this));
    m_imageProviders.append(new TMDbImages(this));
    m_imageProviders.append(new TheTvDbImages(this));

    m_trailerProviders.append(new MovieMaze(this));
    m_trailerProviders.append(new HdTrailers(this));
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
 * @brief Returns the active MediaCenterInterface
 * @return Instance of a MediaCenterInterface
 */
MediaCenterInterface *Manager::mediaCenterInterface()
{
    return m_mediaCenters.at(0);
}

/**
 * @brief Returns the active MediaCenterInterface for TV Shows
 * @return Instance of a MediaCenterinterface
 */
MediaCenterInterface *Manager::mediaCenterInterfaceTvShow()
{
    return m_mediaCentersTvShow.at(0);
}

/**
 * @brief Returns the active MediaCenterInterface for Concerts
 * @return Instance of a MediaCenterinterface
 */
MediaCenterInterface *Manager::mediaCenterInterfaceConcert()
{
    return m_mediaCentersConcert.at(0);
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
 * @brief Returns an instance of the concert file searcher
 * @return Instance of tv show file searcher
 */
ConcertFileSearcher *Manager::concertFileSearcher()
{
    return m_concertFileSearcher;
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
 * @brief Returns a list of all concert scrapers
 * @return List of pointers of concert scrapers
 */
QList<ConcertScraperInterface*> Manager::concertScrapers()
{
    return m_concertScrapers;
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
 * @brief Returns an instance of the ConcertModel
 * @return Instance of the ConcertModel
 */
ConcertModel *Manager::concertModel()
{
    return m_concertModel;
}

/**
 * @brief Returns an instance of the TvShowProxyModel
 * @return Instance of the TvShowProxyModel
 */
TvShowProxyModel *Manager::tvShowProxyModel()
{
    return m_tvShowProxyModel;
}

/**
 * @brief Returns the scraper for the given identifier
 * @param name Identifier of the scraper
 * @return Scraper Interface
 */
ScraperInterface *Manager::getScraperForName(QString name)
{
    if (name == "ofdb") {
        return m_scrapers.at(3);
    } else if (name == "cinefacts") {
        return m_scrapers.at(2);
    } else if (name == "videobuster") {
        return m_scrapers.at(4);
    } else if (name == "imdb") {
        return m_scrapers.at(1);
    } else {
        // default to TMDb
        return m_scrapers.at(0);
    }
}

/**
 * @brief Returns a list of all image providers available for type
 * @param type Type of image
 * @return List of pointers of image providers
 */
QList<ImageProviderInterface*> Manager::imageProviders(int type)
{
    QList<ImageProviderInterface*> providers;
    foreach (ImageProviderInterface* provider, m_imageProviders) {
        if (provider->provides().contains(type))
            providers.append(provider);
    }
    return providers;
}

/**
 * @brief Returns a list of all image providers
 * @return List of pointers of image providers
 */
QList<ImageProviderInterface*> Manager::imageProviders()
{
    return m_imageProviders;
}

/**
 * @brief Manager::fanartTv
 * @return
 */
FanartTv* Manager::fanartTv()
{
    return static_cast<FanartTv*>(m_imageProviders.at(0));
}

/**
 * @brief Manager::database
 * @return
 */
Database *Manager::database()
{
    return m_database;
}

/**
 * @brief Manager::setTvShowFilesWidget
 * @param widget
 */
void Manager::setTvShowFilesWidget(TvShowFilesWidget *widget)
{
    m_tvShowFilesWidget = widget;
}

/**
 * @brief Manager::tvShowFilesWidget
 * @return
 */
TvShowFilesWidget *Manager::tvShowFilesWidget()
{
    return m_tvShowFilesWidget;
}

FileScannerDialog *Manager::fileScannerDialog()
{
    return m_fileScannerDialog;
}

void Manager::setFileScannerDialog(FileScannerDialog *dialog)
{
    m_fileScannerDialog = dialog;
}

QList<TrailerProvider*> Manager::trailerProviders()
{
    return m_trailerProviders;
}
