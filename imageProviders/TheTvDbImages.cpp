#include "TheTvDbImages.h"

/**
 * @brief TheTvDbImages::TheTvDbImages
 * @param parent
 */
TheTvDbImages::TheTvDbImages(QObject *parent)
{
    setParent(parent);
    m_provides << ImageDialogType::TvShowPoster << ImageDialogType::TvShowBackdrop << ImageDialogType::TvShowBanner
               << ImageDialogType::TvShowSeason << ImageDialogType::TvShowThumb;
    m_dummyShow = new TvShow(QString(), this);
    m_dummyEpisode = new TvShowEpisode(QStringList(), m_dummyShow);
    m_tvdb = new TheTvDb(this);
    m_tvdb->loadSettings();
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

    if (type == TypeShowThumbnail)
        m_tvdb->loadTvShowEpisodeData(tvdbId, m_dummyEpisode, infosToLoad);
    else
        m_tvdb->loadTvShowData(tvdbId, m_dummyShow, UpdateShow, infosToLoad);
}

/**
 * @brief Called when the tv show images are downloaded
 * @see TMDbImages::parseTvShowData
 */
void TheTvDbImages::onLoadTvShowDataFinished()
{
    QList<Poster> posters;
    if (m_currentType == TypePoster) {
        posters = m_dummyShow->posters();
    } else if (m_currentType == TypeBackdrop) {
        posters = m_dummyShow->backdrops();
    } else if (m_currentType == TypeBanner) {
        posters = m_dummyShow->banners();
    } else if (m_currentType == TypeSeasonPoster) {
        posters = m_dummyShow->seasonPosters(m_season);
    } else if (m_currentType == TypeShowThumbnail && !m_dummyEpisode->thumbnail().isEmpty()) {
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
    loadTvShowData(tvdbId, TypePoster);
}

/**
 * @brief Load tv show backdrops
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowBackdrops(QString tvdbId)
{
    loadTvShowData(tvdbId, TypeBackdrop);
}

/**
 * @brief Load tv show banners
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowBanners(QString tvdbId)
{
    loadTvShowData(tvdbId, TypeBanner);
}

/**
 * @brief Load tv show thumbs
 * @param tvdbId The TV DB id
 * @param season Season number
 * @param episode Episode number
 */
void TheTvDbImages::tvShowThumb(QString tvdbId, int season, int episode)
{
    m_dummyEpisode->clear();
    m_dummyEpisode->setSeason(season);
    m_dummyEpisode->setEpisode(episode);
    loadTvShowData(tvdbId, TypeShowThumbnail);
}

/**
 * @brief Load tv show season
 * @param tvdbId The TV DB id
 * @param season Season number
 */
void TheTvDbImages::tvShowSeason(QString tvdbId, int season)
{
    m_season = season;
    loadTvShowData(tvdbId, TypeSeasonPoster);
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
