#include "TheTvDbImages.h"

#include "scrapers/tv_show/TheTvDb.h"
#include "settings/Settings.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

TheTvDbImages::TheTvDbImages(QObject* parent)
{
    setParent(parent);
    m_provides = {ImageType::TvShowPoster,
        ImageType::TvShowBackdrop,
        ImageType::TvShowBanner,
        ImageType::TvShowSeasonPoster,
        ImageType::TvShowEpisodeThumb,
        ImageType::TvShowSeasonBanner,
        ImageType::TvShowSeasonBackdrop};
    m_dummyShow = new TvShow(QString(), this);
    m_dummyEpisode = new TvShowEpisode(QStringList(), m_dummyShow);
    m_tvdb = new TheTvDb(this);
    m_searchResultLimit = 0;
    connect(m_tvdb, &TheTvDb::sigSearchDone, this, &TheTvDbImages::onSearchTvShowFinished);
    connect(m_dummyShow, &TvShow::sigLoaded, this, &TheTvDbImages::onLoadTvShowDataFinished);
    connect(m_dummyEpisode, &TvShowEpisode::sigLoaded, this, &TheTvDbImages::onLoadTvShowDataFinished);
}

/**
 * @brief Returns the name of this image provider
 * @return Name of this image provider
 */
QString TheTvDbImages::name() const
{
    return QString("The TV DB");
}

QUrl TheTvDbImages::siteUrl() const
{
    return QUrl("https://www.thetvdb.com");
}

QString TheTvDbImages::identifier() const
{
    return QString("images.thetvdb");
}

/**
 * @brief Returns a list of supported image types
 * @return List of supported image types
 */
QVector<ImageType> TheTvDbImages::provides()
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
 * @brief Searches for a TV show
 * @param searchStr The TV show name/search string
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
void TheTvDbImages::onSearchTvShowFinished(QVector<ScraperSearchResult> results)
{
    if (m_searchResultLimit == 0) {
        emit sigSearchDone(results, {});
    } else {
        emit sigSearchDone(results.mid(0, m_searchResultLimit), {});
    }
}

void TheTvDbImages::loadTvShowData(TvDbId tvdbId, ImageType type)
{
    m_currentType = type;
    m_dummyShow->clear();

    QVector<TvShowScraperInfos> infosToLoad;
    infosToLoad.append(TvShowScraperInfos::Thumbnail);
    infosToLoad.append(TvShowScraperInfos::Banner);
    infosToLoad.append(TvShowScraperInfos::Fanart);
    infosToLoad.append(TvShowScraperInfos::Poster);
    infosToLoad.append(TvShowScraperInfos::SeasonPoster);
    infosToLoad.append(TvShowScraperInfos::SeasonBanner);
    infosToLoad.append(TvShowScraperInfos::SeasonBackdrop);

    if (type == ImageType::TvShowEpisodeThumb) {
        m_tvdb->loadTvShowEpisodeData(tvdbId, m_dummyEpisode, infosToLoad);
    } else {
        m_dummyShow->loadData(tvdbId, m_tvdb, TvShowUpdateType::Show, infosToLoad);
    }
}

/**
 * @brief Called when the TV show images are downloaded
 * @see TMDbImages::parseTvShowData
 */
void TheTvDbImages::onLoadTvShowDataFinished()
{
    QVector<Poster> posters;
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

    emit sigImagesLoaded(posters, {});
}

void TheTvDbImages::tvShowImages(TvShow* show, TvDbId tvdbId, QVector<ImageType> types)
{
    Q_UNUSED(show);
    Q_UNUSED(tvdbId);
    Q_UNUSED(types);
}

/**
 * @brief Load TV show posters
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowPosters(TvDbId tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowPoster);
}

/**
 * @brief Load TV show backdrops
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowBackdrops(TvDbId tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowBackdrop);
}

/**
 * @brief Load TV show banners
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowBanners(TvDbId tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowBanner);
}

/**
 * @brief Load TV show thumbs
 * @param tvdbId The TV DB id
 * @param season Season number
 * @param episode Episode number
 */
void TheTvDbImages::tvShowEpisodeThumb(TvDbId tvdbId, SeasonNumber season, EpisodeNumber episode)
{
    m_dummyEpisode->clear();
    m_dummyEpisode->setSeason(season);
    m_dummyEpisode->setEpisode(episode);
    loadTvShowData(tvdbId, ImageType::TvShowEpisodeThumb);
}

/**
 * @brief Load TV show season
 * @param tvdbId The TV DB id
 * @param season Season number
 */
void TheTvDbImages::tvShowSeason(TvDbId tvdbId, SeasonNumber season)
{
    m_season = season;
    loadTvShowData(tvdbId, ImageType::TvShowSeasonPoster);
}

void TheTvDbImages::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season)
{
    m_season = season;
    loadTvShowData(tvdbId, ImageType::TvShowSeasonBanner);
}

// UNSUPPORTED

void TheTvDbImages::movieImages(Movie* movie, TmdbId tmdbId, QVector<ImageType> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void TheTvDbImages::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

/**
 * @brief Would load movie posters (not supported by fanart.tv)
 */
void TheTvDbImages::moviePosters(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie backdrops
 */
void TheTvDbImages::movieBackdrops(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie logos
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::movieLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TheTvDbImages::movieBanners(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TheTvDbImages::movieThumbs(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie clear arts
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::movieClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load movie cd arts
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::movieCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TheTvDbImages::concertImages(Concert* concert, TmdbId tmdbId, QVector<ImageType> types)
{
    Q_UNUSED(concert);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

/**
 * @brief Would load concert posters (not supported by fanart.tv)
 */
void TheTvDbImages::concertPosters(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert backdrops
 */
void TheTvDbImages::concertBackdrops(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert logos
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::concertLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert clear arts
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::concertClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert cd arts
 * @param tmdbId The Movie DB id
 */
void TheTvDbImages::concertCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load TV show logos
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowLogos(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void TheTvDbImages::tvShowThumbs(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load TV show clear arts
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowClearArts(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

/**
 * @brief Load TV show character arts
 * @param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowCharacterArts(TvDbId tvdbId)
{
    Q_UNUSED(tvdbId);
}

void TheTvDbImages::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season)
{
    Q_UNUSED(season);
    loadTvShowData(tvdbId, ImageType::TvShowSeasonBackdrop);
}

bool TheTvDbImages::hasSettings() const
{
    return false;
}

void TheTvDbImages::loadSettings(const ScraperSettings& settings)
{
    m_tvdb->loadSettings(settings);
}

void TheTvDbImages::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

QWidget* TheTvDbImages::settingsWidget()
{
    return nullptr;
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

void TheTvDbImages::artistImages(Artist* artist, QString mbId, QVector<ImageType> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TheTvDbImages::albumImages(Album* album, QString mbId, QVector<ImageType> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TheTvDbImages::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
