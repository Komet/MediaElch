#include "globals/Manager.h"

#include <QApplication>
#include <QDesktopServices>
#include <QSqlQuery>

#include "globals/Globals.h"
#include "media_centers/KodiXml.h"
#include "media_centers/MediaCenterInterface.h"
#include "scrapers/image/FanartTv.h"
#include "scrapers/image/FanartTvMusic.h"
#include "scrapers/image/FanartTvMusicArtists.h"
#include "scrapers/image/TMDbImages.h"
#include "scrapers/image/TheTvDbImages.h"
#include "scrapers/music/UniversalMusicScraper.h"
#include "scrapers/trailer/HdTrailers.h"

Manager::Manager(QObject* parent) : QObject(parent)
{
    using namespace mediaelch::scraper;

    m_scraperManager = new mediaelch::ScraperManager(this);

    m_movieFileSearcher = new mediaelch::MovieFileSearcher(this);
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

    m_iconFont = new MyIconFont(this);
    m_iconFont->initFontAwesome();
}

Manager* Manager::instance()
{
    static auto* s_instance = new Manager(QApplication::instance());
    return s_instance;
}

mediaelch::ScraperManager& Manager::scrapers()
{
    return *m_scraperManager;
}

/**
 * \brief Returns the active MediaCenterInterface
 * \return Instance of a MediaCenterInterface
 */
MediaCenterInterface* Manager::mediaCenterInterface()
{
    return m_mediaCenters.at(0);
}

/**
 * \brief Returns the active MediaCenterInterface for TV Shows
 * \return Instance of a MediaCenterinterface
 */
MediaCenterInterface* Manager::mediaCenterInterfaceTvShow()
{
    return m_mediaCentersTvShow.at(0);
}

/**
 * \brief Returns the active MediaCenterInterface for Concerts
 * \return Instance of a MediaCenterinterface
 */
MediaCenterInterface* Manager::mediaCenterInterfaceConcert()
{
    return m_mediaCentersConcert.at(0);
}

/**
 * \brief Returns an instance of the movie file searcher
 * \return Instance of movie searcher
 */
mediaelch::MovieFileSearcher* Manager::movieFileSearcher()
{
    return m_movieFileSearcher;
}

/**
 * \brief Returns an instance of the TV show file searcher
 * \return Instance of TV show file searcher
 */
TvShowFileSearcher* Manager::tvShowFileSearcher()
{
    return m_tvShowFileSearcher;
}

/**
 * \brief Returns an instance of the concert file searcher
 * \return Instance of TV show file searcher
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
 * \brief Returns an instance of the MovieModel
 * \return Instance of the MovieModel
 */
MovieModel* Manager::movieModel()
{
    return m_movieModel;
}

/**
 * \brief Returns an instance of the TvShowModel
 * \return Instance of the TvShowModel
 */
TvShowModel* Manager::tvShowModel()
{
    return m_tvShowModel;
}

/**
 * \brief Returns an instance of the ConcertModel
 * \return Instance of the ConcertModel
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
 * \brief Returns a list of all image providers available for type
 * \param type Type of image
 * \return List of pointers of image providers
 */
QVector<mediaelch::scraper::ImageProvider*> Manager::imageProviders(ImageType type)
{
    QVector<mediaelch::scraper::ImageProvider*> providers;
    for (auto* provider : asConst(m_imageProviders)) {
        if (provider->meta().supportedImageTypes.contains(type)) {
            providers.append(provider);
        }
    }
    return providers;
}

QVector<mediaelch::scraper::ImageProvider*> Manager::imageProviders()
{
    return m_imageProviders;
}

mediaelch::scraper::FanartTv* Manager::fanartTv()
{
    return dynamic_cast<mediaelch::scraper::FanartTv*>(m_imageProviders.at(0));
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

QVector<mediaelch::scraper::TrailerProvider*> Manager::trailerProviders()
{
    return m_trailerProviders;
}

MyIconFont* Manager::iconFont()
{
    return m_iconFont;
}
