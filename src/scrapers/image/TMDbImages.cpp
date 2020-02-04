#include "TMDbImages.h"

#include "scrapers/movie/TMDb.h"
#include "settings/Settings.h"

TMDbImages::TMDbImages(QObject* parent)
{
    setParent(parent);
    m_provides = {ImageType::MovieBackdrop, //
        ImageType::MoviePoster,             //
        ImageType::ConcertBackdrop,         //
        ImageType::ConcertPoster};
    m_searchResultLimit = 0;
    m_tmdb = new TMDb(this);
    m_dummyMovie = new Movie(QStringList(), this);
    connect(m_dummyMovie->controller(), &MovieController::sigInfoLoadDone, this, &TMDbImages::onLoadImagesFinished);
    connect(m_tmdb, &TMDb::searchDone, this, &TMDbImages::onSearchMovieFinished);
}

/**
 * @brief Returns the name of this image provider
 * @return Name of this image provider
 */
QString TMDbImages::name() const
{
    return QString("The Movie DB");
}

QUrl TMDbImages::siteUrl() const
{
    return QUrl("https://www.themoviedb.org");
}

QString TMDbImages::identifier() const
{
    return QString("images.tmdb");
}

/**
 * @brief Returns a list of supported image types
 * @return List of supported image types
 */
QVector<ImageType> TMDbImages::provides()
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
void TMDbImages::onSearchMovieFinished(QVector<ScraperSearchResult> results, ScraperSearchError error)
{
    if (m_searchResultLimit == 0) {
        emit sigSearchDone(results, error);
    } else {
        emit sigSearchDone(results.mid(0, m_searchResultLimit), error);
    }
}

/**
 * @brief Load movie posters
 */
void TMDbImages::moviePosters(TmdbId tmdbId)
{
    m_dummyMovie->clear();
    m_imageType = ImageType::MoviePoster;
    QVector<MovieScraperInfos> infos;
    infos << MovieScraperInfos::Poster;
    QMap<MovieScraperInterface*, QString> ids;
    ids.insert(nullptr, tmdbId.toString());
    m_tmdb->loadData(ids, m_dummyMovie, infos);
}

/**
 * @brief Load movie backdrops
 */
void TMDbImages::movieBackdrops(TmdbId tmdbId)
{
    m_dummyMovie->clear();
    m_imageType = ImageType::MovieBackdrop;
    QVector<MovieScraperInfos> infos;
    infos << MovieScraperInfos::Backdrop;
    QMap<MovieScraperInterface*, QString> ids;
    ids.insert(nullptr, tmdbId.toString());
    m_tmdb->loadData(ids, m_dummyMovie, infos);
}

/**
 * @brief Load concert posters
 */
void TMDbImages::concertPosters(TmdbId tmdbId)
{
    moviePosters(tmdbId);
}

/**
 * @brief Load concert backdrops
 */
void TMDbImages::concertBackdrops(TmdbId tmdbId)
{
    movieBackdrops(tmdbId);
}

/**
 * @brief Called when the movie images are downloaded
 */
void TMDbImages::onLoadImagesFinished()
{
    QVector<Poster> posters;
    if (m_imageType == ImageType::MovieBackdrop) {
        posters = m_dummyMovie->images().backdrops();
    } else if (m_imageType == ImageType::MoviePoster) {
        posters = m_dummyMovie->images().posters();
    }

    emit sigImagesLoaded(posters, {});
}

void TMDbImages::movieImages(Movie* movie, TmdbId tmdbId, QVector<ImageType> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

/**
 * @brief Load movie logos
 * @param tmdbId The Movie DB id
 */
void TMDbImages::movieLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TMDbImages::movieBanners(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TMDbImages::movieThumbs(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie clear arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::movieClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie cd arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::movieCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TMDbImages::concertImages(Concert* concert, TmdbId tmdbId, QVector<ImageType> types)
{
    Q_UNUSED(concert);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

/**
 * @brief Load concert logos
 * @param tmdbId The Movie DB id
 */
void TMDbImages::concertLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert clear arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::concertClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert cd arts
 * @param tmdbId The Movie DB id
 */
void TMDbImages::concertCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Searches for a TV show
 * @param searchStr Search term
 * @param limit Number of results, if zero, all results are returned
 */
void TMDbImages::searchTvShow(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void TMDbImages::tvShowImages(TvShow* show, TvDbId tvdbId, QVector<ImageType> types)
{
    Q_UNUSED(show);
    Q_UNUSED(tvdbId);
    Q_UNUSED(types);
}

/**
 * @brief Load TV show posters
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowPosters(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load TV show backdrops
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowBackdrops(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load TV show logos
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowLogos(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void TMDbImages::tvShowThumbs(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load TV show clear arts
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowClearArts(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load TV show character arts
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowCharacterArts(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load TV show banners
 * @param tvdbId The TV DB id
 */
void TMDbImages::tvShowBanners(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load TV show thumbs
 * @param tvdbId The TV DB id
 * @param season Season number
 * @param episode Episode number
 */
void TMDbImages::tvShowEpisodeThumb(TvDbId tvdbId, SeasonNumber season, EpisodeNumber episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

/**
 * @brief Load TV show season
 * @param tvdbId The TV DB id
 * @param season Season number
 */
void TMDbImages::tvShowSeason(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void TMDbImages::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void TMDbImages::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void TMDbImages::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

bool TMDbImages::hasSettings() const
{
    return false;
}

void TMDbImages::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void TMDbImages::loadSettings(const ScraperSettings& settings)
{
    m_tmdb->loadSettings(settings);
}

QWidget* TMDbImages::settingsWidget()
{
    return nullptr;
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

void TMDbImages::artistImages(Artist* artist, QString mbId, QVector<ImageType> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TMDbImages::albumImages(Album* album, QString mbId, QVector<ImageType> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TMDbImages::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
