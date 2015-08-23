#include "globals/Manager.h"

#include <QApplication>
#include <QDesktopServices>
#include <QSqlQuery>
#include "globals/Globals.h"
#include "imageProviders/Coverlib.h"
#include "imageProviders/FanartTv.h"
#include "imageProviders/FanartTvMusic.h"
#include "imageProviders/FanartTvMusicArtists.h"
#include "imageProviders/MediaPassionImages.h"
#include "imageProviders/TMDbImages.h"
#include "imageProviders/TheTvDbImages.h"
#include "mediaCenterPlugins/XbmcXml.h"
#include "scrapers/AdultDvdEmpire.h"
#include "scrapers/AEBN.h"
#include "scrapers/Cinefacts.h"
#include "scrapers/CustomMovieScraper.h"
#include "scrapers/HotMovies.h"
#include "scrapers/IMDB.h"
#include "scrapers/MediaPassion.h"
#include "scrapers/OFDb.h"
#include "scrapers/UniversalMusicScraper.h"
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
    m_scrapers.append(Manager::constructNativeScrapers(this));
    m_scrapers.append(CustomMovieScraper::instance(this));
    m_scrapers.append(new AEBN(parent));
    m_scrapers.append(new HotMovies(parent));
    m_scrapers.append(new AdultDvdEmpire(parent));
    m_tvScrapers.append(new TheTvDb(this));
    m_concertScrapers.append(new TMDbConcerts(this));
    m_musicScrapers.append(new UniversalMusicScraper(this));
    m_movieFileSearcher = new MovieFileSearcher(this);
    m_tvShowFileSearcher = new TvShowFileSearcher(this);
    m_concertFileSearcher = new ConcertFileSearcher(this);
    m_musicFileSearcher = new MusicFileSearcher(this);
    m_movieModel = new MovieModel(this);
    m_tvShowModel = new TvShowModel(this);
    m_tvShowProxyModel = new TvShowProxyModel(this);
    m_concertModel = new ConcertModel(this);
    m_musicModel = new MusicModel(this);
    m_database = new Database(this);

    m_mediaCenters.append(new XbmcXml(this));
    m_mediaCentersTvShow.append(new XbmcXml(this));
    m_mediaCentersConcert.append(new XbmcXml(this));

    m_imageProviders.append(new FanartTv(this));
    m_imageProviders.append(new FanartTvMusic(this));
    m_imageProviders.append(new FanartTvMusicArtists(this));
    m_imageProviders.append(new MediaPassionImages(this));
    m_imageProviders.append(new TMDbImages(this));
    m_imageProviders.append(new TheTvDbImages(this));
    m_imageProviders.append(new Coverlib(this));

    m_trailerProviders.append(new MovieMaze(this));
    m_trailerProviders.append(new HdTrailers(this));

    m_tvTunes = new TvTunes(this);

    m_iconFont = new MyIconFont(this);
    m_iconFont->initFontAwesome();

    qRegisterMetaType<Image*>("Image*");
    qRegisterMetaType<ImageModel*>("ImageModel*");
    qRegisterMetaType<ImageProxyModel*>("ImageProxyModel*");
    qRegisterMetaType<Album*>("Album*");
    qRegisterMetaType<Artist*>("Artist*");
    qRegisterMetaType<MusicModelItem*>("MusicModelItem*");
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

MusicFileSearcher *Manager::musicFileSearcher()
{
    return m_musicFileSearcher;
}

/**
 * @brief Returns a list of all movie scrapers
 * @return List of pointers of movie scrapers
 */
QList<ScraperInterface*> Manager::scrapers()
{
    return m_scrapers;
}

ScraperInterface* Manager::scraper(const QString &identifier)
{
    foreach (ScraperInterface *scraper, m_scrapers) {
        if (scraper->identifier() == identifier)
            return scraper;
    }

    return 0;
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

QList<MusicScraperInterface*> Manager::musicScrapers()
{
    return m_musicScrapers;
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

MusicModel *Manager::musicModel()
{
    return m_musicModel;
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

void Manager::setMusicFilesWidget(MusicFilesWidget *widget)
{
    m_musicFilesWidget = widget;
}

MusicFilesWidget *Manager::musicFilesWidget()
{
    return m_musicFilesWidget;
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

TvTunes* Manager::tvTunes()
{
    return m_tvTunes;
}

QList<ScraperInterface*> Manager::constructNativeScrapers(QObject *parent)
{
    QList<ScraperInterface*> nativeScrapers;
    nativeScrapers.append(new TMDb(parent));
    nativeScrapers.append(new IMDB(parent));
    nativeScrapers.append(new MediaPassion(parent));
    nativeScrapers.append(new Cinefacts(parent));
    nativeScrapers.append(new OFDb(parent));
    nativeScrapers.append(new VideoBuster(parent));
    return nativeScrapers;
}

MyIconFont *Manager::iconFont()
{
    return m_iconFont;
}
