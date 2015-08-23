#include "TheTvDbImages.h"

#include "settings/Settings.h"

/**
 * @brief TheTvDbImages::TheTvDbImages
 * @param parent
 */
TheTvDbImages::TheTvDbImages(QObject *parent)
{
    setParent(parent);
    m_provides << ImageType::TvShowPoster << ImageType::TvShowBackdrop << ImageType::TvShowBanner
               << ImageType::TvShowSeasonPoster << ImageType::TvShowEpisodeThumb << ImageType::TvShowSeasonBanner
               << ImageType::TvShowSeasonBackdrop;
    m_dummyShow = new TvShow(QString(), this);
    m_dummyEpisode = new TvShowEpisode(QStringList(), m_dummyShow);
    m_tvdb = new TheTvDb(this);
    m_searchResultLimit = 0;
    connect(m_tvdb, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchTvShowFinished(QList<ScraperSearchResult>)));
    connect(m_dummyShow, SIGNAL(sigLoaded(TvShow*)), this, SLOT(onLoadTvShowDataFinished()));
    connect(m_dummyEpisode, SIGNAL(sigLoaded()), this, SLOT(onLoadTvShowDataFinished()));
}

/**
 * @brief Returns the name of this image provider
 * @return Name of this image provider
 */
QString TheTvDbImages::name()
{
    return QString("The TV DB");
}

QUrl TheTvDbImages::siteUrl()
{
    return QUrl("http://www.thetvdb.com");
}

QString TheTvDbImages::identifier()
{
    return QString("images.thetvdb");
}

/**
 * @brief Returns a list of supported image types
 * @return List of supported image types
 */
QList<int> TheTvDbImages::provides()
{
    return m_provides;
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @param limit Number of results, if zero, all results are returned
 */
void TheTvDbImages::searchMovie(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

/**
 * @brief Searches for a concert
 * @param searchStr The Concert name/search string
 * @param limit Number of results, if zero, all results are returned
 */
void TheTvDbImages::searchConcert(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

/**
 * @brief Searches for a tv show
 * @param searchStr The tv show name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see TheTvDbImages::onSearchTvShowFinished
 */
void TheTvDbImages::searchTvShow(QString searchStr, int limit)
{
    m_searchResultLimit = limit;
    m_tvdb->search(searchStr);
}

/**
 * @brief TheTvDbImages::onSearchTvShowFinished
 * @param results Result list
 */
void TheTvDbImages::onSearchTvShowFinished(QList<ScraperSearchResult> results)
{
    if (m_searchResultLimit == 0)
        emit sigSearchDone(results);
    else
        emit sigSearchDone(results.mid(0, m_searchResultLimit));
}

/**
 * @brief FanartTv::loadTvShowData
 * @param tvdbId
 * @param type
 */
void TheTvDbImages::loadTvShowData(QString tvdbId, int type)
{
    m_currentType = type;
    m_dummyShow->clear();

    QList<int> infosToLoad;
    infosToLoad.append(TvShowScraperInfos::Thumbnail);
    infosToLoad.append(TvShowScraperInfos::Banner);
    infosToLoad.append(TvShowScraperInfos::Fanart);
    infosToLoad.append(TvShowScraperInfos::Poster);
    infosToLoad.append(TvShowScraperInfos::SeasonPoster);
    infosToLoad.append(TvShowScraperInfos::SeasonBanner);
    infosToLoad.append(TvShowScraperInfos::SeasonBackdrop);

    if (type == ImageType::TvShowEpisodeThumb)
        m_tvdb->loadTvShowEpisodeData(tvdbId, m_dummyEpisode, infosToLoad);
    else
        m_dummyShow->loadData(tvdbId, m_tvdb, UpdateShow, infosToLoad);
}

/**
 * @brief Called when the tv show images are downloaded
 * @see TMDbImages::parseTvShowData
 */
void TheTvDbImages::onLoadTvShowDataFinished()
{
    QList<Poster> posters;
    if (m_currentType == ImageType::TvShowPoster) {
        posters = m_dummyShow->posters();
    } else if (m_currentType == ImageType::TvShowBackdrop) {
        posters = m_dummyShow->backdrops();
    } else if (m_currentType == ImageType::TvShowBanner) {
        posters = m_dummyShow->banners();
    } else if (m_currentType == ImageType::TvShowSeasonPoster) {
        posters = m_dummyShow->seasonPosters(m_season);
    } else if (m_currentType == ImageType::TvShowSeasonBackdrop) {
        posters = m_dummyShow->backdrops();
    } else if (m_currentType == ImageType::TvShowSeasonBanner) {
        posters = m_dummyShow->seasonBanners(m_season, true);
        posters << m_dummyShow->banners();
    } else if (m_currentType == ImageType::TvShowEpisodeThumb && !m_dummyEpisode->thumbnail().isEmpty()) {
        Poster p;
        p.thumbUrl = m_dummyEpisode->thumbnail();
        p.originalUrl = m_dummyEpisode->thumbnail();
        posters << p;
    }

    emit sigImagesLoaded(posters);
}

/**
 * @brief TheTvDbImages::tvShowImages
 * @param show
 * @param tvdbId
 * @param types
 */
void TheTvDbImages::tvShowImages(TvShow *show, QString tvdbId, QList<int> types)
{
    Q_UNUSED(show);
    Q_UNUSED(tvdbId);
    Q_UNUSED(types);
}

/**
 * @brief Load tv show posters
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowPosters(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowPoster);
}

/**
 * @brief Load tv show backdrops
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowBackdrops(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowBackdrop);
}

/**
 * @brief Load tv show banners
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowBanners(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowBanner);
}

/**
 * @brief Load tv show thumbs
 * @param tvdbId The TV DB id
 * @param season Season number
 * @param episode Episode number
 */
void TheTvDbImages::tvShowEpisodeThumb(QString tvdbId, int season, int episode)
{
    m_dummyEpisode->clear();
    m_dummyEpisode->setSeason(season);
    m_dummyEpisode->setEpisode(episode);
    loadTvShowData(tvdbId, ImageType::TvShowEpisodeThumb);
}

/**
 * @brief Load tv show season
 * @param tvdbId The TV DB id
 * @param season Season number
 */
void TheTvDbImages::tvShowSeason(QString tvdbId, int season)
{
    m_season = season;
    loadTvShowData(tvdbId, ImageType::TvShowSeasonPoster);
}

void TheTvDbImages::tvShowSeasonBanners(QString tvdbId, int season)
{
    m_season = season;
    loadTvShowData(tvdbId, ImageType::TvShowSeasonBanner);
}

// UNSUPPORTED

/**
 * @brief TheTvDbImages::movieImages
 * @param movie
 * @param tmdbId
 * @param types
 */
void TheTvDbImages::movieImages(Movie *movie, QString tmdbId, QList<int> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void TheTvDbImages::tvShowSeasonThumbs(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

/**
 * @brief Would load movie posters (not supported by fanart.tv)
 * @param tmdbId
 */
void TheTvDbImages::moviePosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie backdrops
 * @param tmdbId
 */
void TheTvDbImages::movieBackdrops(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie logos
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::movieLogos(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TheTvDbImages::movieBanners(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TheTvDbImages::movieThumbs(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie clear arts
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::movieClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie cd arts
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::movieCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief TheTvDbImages::concertImages
 * @param concert
 * @param tmdbId
 * @param types
 */
void TheTvDbImages::concertImages(Concert *concert, QString tmdbId, QList<int> types)
{
    Q_UNUSED(concert);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

/**
 * @brief Would load concert posters (not supported by fanart.tv)
 * @param tmdbId
 */
void TheTvDbImages::concertPosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert backdrops
 * @param tmdbId
 */
void TheTvDbImages::concertBackdrops(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert logos
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::concertLogos(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert clear arts
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::concertClearArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert cd arts
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::concertCdArts(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load tv show logos
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowLogos(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void TheTvDbImages::tvShowThumbs(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load tv show clear arts
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowClearArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load tv show character arts
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowCharacterArts(QString tvdbId)
{
    Q_UNUSED(tvdbId);
}

void TheTvDbImages::tvShowSeasonBackdrops(QString tvdbId, int season)
{
    Q_UNUSED(season);
    loadTvShowData(tvdbId, ImageType::TvShowSeasonBackdrop);
}

bool TheTvDbImages::hasSettings()
{
    return false;
}

void TheTvDbImages::loadSettings(QSettings &settings)
{
    m_tvdb->loadSettings(settings);
}

void TheTvDbImages::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

QWidget* TheTvDbImages::settingsWidget()
{
    return 0;
}

void TheTvDbImages::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(artistName);
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void TheTvDbImages::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void TheTvDbImages::artistFanarts(QString mbId)
{
    Q_UNUSED(mbId);
}

void TheTvDbImages::artistLogos(QString mbId)
{
    Q_UNUSED(mbId);
}

void TheTvDbImages::artistThumbs(QString mbId)
{
    Q_UNUSED(mbId);
}

void TheTvDbImages::albumCdArts(QString mbId)
{
    Q_UNUSED(mbId);
}

void TheTvDbImages::albumThumbs(QString mbId)
{
    Q_UNUSED(mbId);
}

void TheTvDbImages::artistImages(Artist *artist, QString mbId, QList<int> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TheTvDbImages::albumImages(Album *album, QString mbId, QList<int> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TheTvDbImages::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
