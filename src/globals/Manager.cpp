#include "globals/Manager.h"

#include <QApplication>
#include <QDesktopServices>
#include <QSqlQuery>

#include "globals/Globals.h"
#include "media_centers/KodiXml.h"
#include "media_centers/MediaCenterInterface.h"
#include "scrapers/concert/TMDbConcerts.h"
#include "scrapers/image/FanartTv.h"
#include "scrapers/image/FanartTvMusic.h"
#include "scrapers/image/FanartTvMusicArtists.h"
#include "scrapers/image/TMDbImages.h"
#include "scrapers/image/TheTvDbImages.h"
#include "scrapers/movie/AEBN.h"
#include "scrapers/movie/AdultDvdEmpire.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/HotMovies.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/OFDb.h"
#include "scrapers/movie/TMDb.h"
#include "scrapers/movie/VideoBuster.h"
#include "scrapers/music/UniversalMusicScraper.h"
#include "scrapers/trailer/HdTrailers.h"
#include "scrapers/tv_show/TheTvDb.h"

Manager::Manager(QObject* parent) : QObject(parent)
{
    m_scrapers.append(Manager::constructMovieScrapers(this));
    m_scrapers.append(CustomMovieScraper::instance(this));

    m_tvScrapers.append(new TheTvDb(this));
    m_concertScrapers.append(new TMDbConcerts(this));
    m_musicScrapers.append(new UniversalMusicScraper(this));
    m_movieFileSearcher = new MovieFileSearcher(this);
    m_tvShowFileSearcher = new TvShowFileSearcher(this);
    m_concertFileSearcher = new ConcertFileSearcher(this);
    m_musicFileSearcher = new MusicFileSearcher(this);
    m_movieModel = new MovieModel(this);
    m_tvShowModel = new TvShowModel(this);
    m_concertModel = new ConcertModel(this);
    m_musicModel = new MusicModel(this);
    m_database = new Database(this);

    m_mediaCenters.append(new KodiXml(this));
    m_mediaCentersTvShow.append(new KodiXml(this));
    m_mediaCentersConcert.append(new KodiXml(this));

    m_imageProviders.append(new FanartTv(this));
    m_imageProviders.append(new FanartTvMusic(this));
    m_imageProviders.append(new FanartTvMusicArtists(this));
    m_imageProviders.append(new TMDbImages(this));
    m_imageProviders.append(new TheTvDbImages(this));

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

Manager::~Manager() = default;

Manager* Manager::instance()
{
    static auto* s_instance = new Manager(QApplication::instance());
    return s_instance;
}

/**
 * @brief Returns the active MediaCenterInterface
 * @return Instance of a MediaCenterInterface
 */
MediaCenterInterface* Manager::mediaCenterInterface()
{
    return m_mediaCenters.at(0);
}

/**
 * @brief Returns the active MediaCenterInterface for TV Shows
 * @return Instance of a MediaCenterinterface
 */
MediaCenterInterface* Manager::mediaCenterInterfaceTvShow()
{
    return m_mediaCentersTvShow.at(0);
}

/**
 * @brief Returns the active MediaCenterInterface for Concerts
 * @return Instance of a MediaCenterinterface
 */
MediaCenterInterface* Manager::mediaCenterInterfaceConcert()
{
    return m_mediaCentersConcert.at(0);
}

/**
 * @brief Returns an instance of the movie file searcher
 * @return Instance of movie searcher
 */
MovieFileSearcher* Manager::movieFileSearcher()
{
    return m_movieFileSearcher;
}

/**
 * @brief Returns an instance of the TV show file searcher
 * @return Instance of TV show file searcher
 */
TvShowFileSearcher* Manager::tvShowFileSearcher()
{
    return m_tvShowFileSearcher;
}

/**
 * @brief Returns an instance of the concert file searcher
 * @return Instance of TV show file searcher
 */
ConcertFileSearcher* Manager::concertFileSearcher()
{
    return m_concertFileSearcher;
}

MusicFileSearcher* Manager::musicFileSearcher()
{
    return m_musicFileSearcher;
}

/**
 * @brief Returns a list of all movie scrapers
 * @return List of pointers of movie scrapers
 */
QVector<MovieScraperInterface*> Manager::movieScrapers()
{
    return m_scrapers;
}

MovieScraperInterface* Manager::scraper(const QString& identifier)
{
    for (auto* scraper : m_scrapers) {
        if (scraper->identifier() == identifier) {
            return scraper;
        }
    }

    return nullptr;
}

/**
 * @brief Returns a list of all tv scrapers
 * @return List of pointers of tv scrapers
 */
QVector<TvScraperInterface*> Manager::tvScrapers()
{
    return m_tvScrapers;
}

/**
 * @brief Returns a list of all concert scrapers
 * @return List of pointers of concert scrapers
 */
QVector<ConcertScraperInterface*> Manager::concertScrapers()
{
    return m_concertScrapers;
}

QVector<MusicScraperInterface*> Manager::musicScrapers()
{
    return m_musicScrapers;
}

/**
 * @brief Returns an instance of the MovieModel
 * @return Instance of the MovieModel
 */
MovieModel* Manager::movieModel()
{
    return m_movieModel;
}

/**
 * @brief Returns an instance of the TvShowModel
 * @return Instance of the TvShowModel
 */
TvShowModel* Manager::tvShowModel()
{
    return m_tvShowModel;
}

/**
 * @brief Returns an instance of the ConcertModel
 * @return Instance of the ConcertModel
 */
ConcertModel* Manager::concertModel()
{
    return m_concertModel;
}

MusicModel* Manager::musicModel()
{
    return m_musicModel;
}

/**
 * @brief Returns a list of all image providers available for type
 * @param type Type of image
 * @return List of pointers of image providers
 */
QVector<ImageProviderInterface*> Manager::imageProviders(ImageType type)
{
    QVector<ImageProviderInterface*> providers;
    for (auto provider : m_imageProviders) {
        if (provider->provides().contains(type)) {
            providers.append(provider);
        }
    }
    return providers;
}

/**
 * @brief Returns a list of all image providers
 * @return List of pointers of image providers
 */
QVector<ImageProviderInterface*> Manager::imageProviders()
{
    return m_imageProviders;
}

FanartTv* Manager::fanartTv()
{
    return dynamic_cast<FanartTv*>(m_imageProviders.at(0));
}

Database* Manager::database()
{
    return m_database;
}

void Manager::setTvShowFilesWidget(TvShowFilesWidget* widget)
{
    m_tvShowFilesWidget = widget;
}

TvShowFilesWidget* Manager::tvShowFilesWidget()
{
    return m_tvShowFilesWidget;
}

void Manager::setMusicFilesWidget(MusicFilesWidget* widget)
{
    m_musicFilesWidget = widget;
}

MusicFilesWidget* Manager::musicFilesWidget()
{
    return m_musicFilesWidget;
}

FileScannerDialog* Manager::fileScannerDialog()
{
    return m_fileScannerDialog;
}

void Manager::setFileScannerDialog(FileScannerDialog* dialog)
{
    m_fileScannerDialog = dialog;
}

QVector<TrailerProvider*> Manager::trailerProviders()
{
    return m_trailerProviders;
}

TvTunes* Manager::tvTunes()
{
    return m_tvTunes;
}

QVector<MovieScraperInterface*> Manager::constructNativeScrapers(QObject* scraperParent)
{
    QVector<MovieScraperInterface*> nativeScrapers;
    nativeScrapers.append(new TMDb(scraperParent));
    nativeScrapers.append(new IMDB(scraperParent));
    nativeScrapers.append(new OFDb(scraperParent));
    nativeScrapers.append(new VideoBuster(scraperParent));
    return nativeScrapers;
}

QVector<MovieScraperInterface*> Manager::constructMovieScrapers(QObject* scraperParent)
{
    auto scrapers = Manager::constructNativeScrapers(scraperParent);
    scrapers.append(new AEBN(scraperParent));
    scrapers.append(new HotMovies(scraperParent));
    scrapers.append(new AdultDvdEmpire(scraperParent));

    return scrapers;
}

MyIconFont* Manager::iconFont()
{
    return m_iconFont;
}
