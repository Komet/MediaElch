#include "TMDbImages.h"

#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include "scrapers/TMDb.h"
#include "settings/Settings.h"

/**
 * @brief TMDbImages::TMDbImages
 * @param parent
 */
TMDbImages::TMDbImages(QObject *parent)
{
    setParent(parent);
    m_provides << ImageType::MovieBackdrop << ImageType::MoviePoster
               << ImageType::ConcertBackdrop << ImageType::ConcertPoster;
    m_searchResultLimit = 0;
    m_tmdb = new TMDb(this);
    m_dummyMovie = new Movie(QStringList(), this);
    connect(m_dummyMovie->controller(), SIGNAL(sigInfoLoadDone(Movie*)), this, SLOT(onLoadImagesFinished()));
    connect(m_tmdb, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchMovieFinished(QList<ScraperSearchResult>)));
}

/**
 * @brief Returns the name of this image provider
 * @return Name of this image provider
 */
QString TMDbImages::name()
{
    return QString("The Movie DB");
}

QUrl TMDbImages::siteUrl()
{
    return QUrl("https://www.themoviedb.org");
}

QString TMDbImages::identifier()
{
    return QString("images.tmdb");
}

/**
 * @brief Returns a list of supported image types
 * @return List of supported image types
 */
QList<int> TMDbImages::provides()
{
    return m_provides;
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see TMDbImages::onSearchMovieFinished
 */
void TMDbImages::searchMovie(QString searchStr, int limit)
{
    m_searchResultLimit = limit;
    m_tmdb->search(searchStr);
}

/**
 * @brief Searches for a concert
 * @param searchStr The concert name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see TMDbImages::searchMovie
 */
void TMDbImages::searchConcert(QString searchStr, int limit)
{
    searchMovie(searchStr, limit);
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "sigSearchDone" if there are no more pages in the result set
 * @param results List of results from scraper
 * @see TMDb::parseSearch
 */
void TMDbImages::onSearchMovieFinished(QList<ScraperSearchResult> results)
{
    qDebug() << "Entered";
    if (m_searchResultLimit == 0)
        emit sigSearchDone(results);
    else
        emit sigSearchDone(results.mid(0, m_searchResultLimit));
}

/**
 * @brief Load movie posters
 * @param tmdbId
 */
void TMDbImages::moviePosters(QString tmdbId)
{
    m_dummyMovie->clear();
    m_imageType = ImageType::MoviePoster;
    QList<int> infos;
    infos << MovieScraperInfos::Poster;
    QMap<ScraperInterface*, QString> ids;
    ids.insert(0, tmdbId);
    m_tmdb->loadData(ids, m_dummyMovie, infos);
}

/**
 * @brief Load movie backdrops
 * @param tmdbId
 */
void TMDbImages::movieBackdrops(QString tmdbId)
{
    m_dummyMovie->clear();
    m_imageType = ImageType::MovieBackdrop;
    QList<int> infos;
    infos << MovieScraperInfos::Backdrop;
    QMap<ScraperInterface*, QString> ids;
    ids.insert(0, tmdbId);
    m_tmdb->loadData(ids, m_dummyMovie, infos);
}

/**
 * @brief Load concert posters
 * @param tmdbId
 */
void TMDbImages::concertPosters(QString tmdbId)
{
    moviePosters(tmdbId);
}

/**
 * @brief Load concert backdrops
 * @param tmdbId
 */
void TMDbImages::concertBackdrops(QString tmdbId)
{
    movieBackdrops(tmdbId);
}

/**
 * @brief Called when the movie images are downloaded
 */
void TMDbImages::onLoadImagesFinished()
{
    QList<Poster> posters;
    if (m_imageType == ImageType::MovieBackdrop)
        posters = m_dummyMovie->backdrops();
    else if (m_imageType == ImageType::MoviePoster)
        posters = m_dummyMovie->posters();

    emit sigImagesLoaded(posters);
}

/**
 * @brief TMDbImages::movieImages
 * @param movie
 * @param tmdbId
 * @param types
 */
void TMDbImages::movieImages(Movie *movie, QString tmdbId, QList<int> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

/**
 * @brief Load movie logos
 * @param tmdbId The Movie DB id
 */
void TMDbImages::movieLogos(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TMDbImages::movieBanners(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TMDbImages::movieThumbs(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie clear arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::movieClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie cd arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::movieCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief TMDbImages::concertImages
 * @param concert
 * @param tmdbId
 * @param types
 */
void TMDbImages::concertImages(Concert *concert, QString tmdbId, QList<int> types)
{
    Q_UNUSED(concert);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

/**
 * @brief Load concert logos
 * @param tmdbId The Movie DB id
 */
void TMDbImages::concertLogos(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert clear arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::concertClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert cd arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::concertCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Searches for a tv show
 * @param searchStr Search term
 * @param limit Number of results, if zero, all results are returned
 */
void TMDbImages::searchTvShow(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

/**
 * @brief TMDbImages::tvShowImages
 * @param show
 * @param tvdbId
 * @param types
 */
void TMDbImages::tvShowImages(TvShow *show, QString tvdbId, QList<int> types)
{
    Q_UNUSED(show);
    Q_UNUSED(tvdbId);
    Q_UNUSED(types);
}

/**
 * @brief Load tv show posters
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowPosters(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load tv show backdrops
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowBackdrops(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load tv show logos
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowLogos(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void TMDbImages::tvShowThumbs(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load tv show clear arts
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowClearArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load tv show character arts
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowCharacterArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load tv show banners
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowBanners(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load tv show thumbs
 * @param tvdbId The TV DB id
 * @param season Season number
 * @param episode Episode number
 */
void TMDbImages::tvShowEpisodeThumb(QString tvdbId, int season, int episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

/**
 * @brief Load tv show season
 * @param tvdbId The TV DB id
 * @param season Season number
 */
void TMDbImages::tvShowSeason(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void TMDbImages::tvShowSeasonBanners(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void TMDbImages::tvShowSeasonThumbs(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void TMDbImages::tvShowSeasonBackdrops(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

bool TMDbImages::hasSettings()
{
    return false;
}

void TMDbImages::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

void TMDbImages::loadSettings(QSettings &settings)
{
    m_tmdb->loadSettings(settings);
}

QWidget* TMDbImages::settingsWidget()
{
    return 0;
}

void TMDbImages::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(artistName);
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void TMDbImages::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void TMDbImages::artistFanarts(QString mbId)
{
    Q_UNUSED(mbId);
}

void TMDbImages::artistLogos(QString mbId)
{
    Q_UNUSED(mbId);
}

void TMDbImages::artistThumbs(QString mbId)
{
    Q_UNUSED(mbId);
}

void TMDbImages::albumCdArts(QString mbId)
{
    Q_UNUSED(mbId);
}

void TMDbImages::albumThumbs(QString mbId)
{
    Q_UNUSED(mbId);
}

void TMDbImages::artistImages(Artist *artist, QString mbId, QList<int> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TMDbImages::albumImages(Album *album, QString mbId, QList<int> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TMDbImages::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
