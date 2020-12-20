#include "TMDbImages.h"

#include "scrapers/movie/TMDb.h"
#include "settings/Settings.h"

namespace mediaelch {
namespace scraper {

TMDbImages::TMDbImages(QObject* parent)
{
    setParent(parent);
    m_provides = {ImageType::MovieBackdrop, //
        ImageType::MoviePoster,             //
        ImageType::ConcertBackdrop,         //
        ImageType::ConcertPoster};
    m_searchResultLimit = 0;
    m_tmdb = new mediaelch::scraper::TMDb(this);
    m_dummyMovie = new Movie({}, this);

    m_supportedLanguages = {"ar-AE",
        "ar-SA",
        "be-BY",
        "bg-BG",
        "bn-BD",
        "ca-ES",
        "ch-GU",
        "cn-CN",
        "cs-CZ",
        "da-DK",
        "de-DE",
        "de-AT",
        "de-CH",
        "el-GR",
        "en-AU",
        "en-CA",
        "en-GB",
        "en-NZ",
        "en-US",
        "eo-EO",
        "es-ES",
        "es-MX",
        "et-EE",
        "eu-ES",
        "fa-IR",
        "fi-FI",
        "fr-CA",
        "fr-FR",
        "gl-ES",
        "he-IL",
        "hi-IN",
        "hu-HU",
        "id-ID",
        "it-IT",
        "ja-JP",
        "ka-GE",
        "kk-KZ",
        "kn-IN",
        "ko-KR",
        "lt-LT",
        "lv-LV",
        "ml-IN",
        "ms-MY",
        "ms-SG",
        "nb-NO",
        "nl-NL",
        "no-NO",
        "pl-PL",
        "pt-BR",
        "pt-PT",
        "ro-RO",
        "ru-RU",
        "si-LK",
        "sk-SK",
        "sl-SI",
        "sq-AL",
        "sr-RS",
        "sv-SE",
        "ta-IN",
        "te-IN",
        "th-TH",
        "tl-PH",
        "tr-TR",
        "uk-UA",
        "vi-VN",
        "zh-CN",
        "zh-HK",
        "zh-TW",
        "zu-ZA"};

    connect(m_dummyMovie->controller(), &MovieController::sigInfoLoadDone, this, &TMDbImages::onLoadImagesFinished);
    connect(m_tmdb, &mediaelch::scraper::TMDb::searchDone, this, &TMDbImages::onSearchMovieFinished);
}

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

mediaelch::Locale TMDbImages::defaultLanguage()
{
    return mediaelch::Locale::English;
}

const QVector<mediaelch::Locale>& TMDbImages::supportedLanguages()
{
    return m_supportedLanguages;
}

/**
 * \brief Returns a list of supported image types
 * \return List of supported image types
 */
QSet<ImageType> TMDbImages::provides()
{
    return m_provides;
}

/**
 * \brief Searches for a movie
 * \param searchStr The Movie name/search string
 * \param limit Number of results, if zero, all results are returned
 * \see TMDbImages::onSearchMovieFinished
 */
void TMDbImages::searchMovie(QString searchStr, int limit)
{
    m_searchResultLimit = limit;
    m_tmdb->search(searchStr);
}

/**
 * \brief Searches for a concert
 * \param searchStr The concert name/search string
 * \param limit Number of results, if zero, all results are returned
 * \see TMDbImages::searchMovie
 */
void TMDbImages::searchConcert(QString searchStr, int limit)
{
    searchMovie(searchStr, limit);
}

/**
 * \brief Called when the search result was downloaded
 *        Emits "sigSearchDone" if there are no more pages in the result set
 * \param results List of results from scraper
 * \see TMDb::parseSearch
 */
void TMDbImages::onSearchMovieFinished(QVector<ScraperSearchResult> results, ScraperError error)
{
    if (m_searchResultLimit == 0) {
        emit sigSearchDone(results, error);
    } else {
        emit sigSearchDone(results.mid(0, m_searchResultLimit), error);
    }
}

/**
 * \brief Load movie posters
 */
void TMDbImages::moviePosters(TmdbId tmdbId)
{
    m_dummyMovie->clear();
    m_imageType = ImageType::MoviePoster;
    QSet<MovieScraperInfo> infos;
    infos << MovieScraperInfo::Poster;
    QHash<mediaelch::scraper::MovieScraper*, QString> ids;
    ids.insert(nullptr, tmdbId.toString());
    m_tmdb->loadData(ids, m_dummyMovie, infos);
}

/**
 * \brief Load movie backdrops
 */
void TMDbImages::movieBackdrops(TmdbId tmdbId)
{
    m_dummyMovie->clear();
    m_imageType = ImageType::MovieBackdrop;
    QSet<MovieScraperInfo> infos;
    infos << MovieScraperInfo::Backdrop;
    QHash<mediaelch::scraper::MovieScraper*, QString> ids;
    ids.insert(nullptr, tmdbId.toString());
    m_tmdb->loadData(ids, m_dummyMovie, infos);
}

/**
 * \brief Load concert posters
 */
void TMDbImages::concertPosters(TmdbId tmdbId)
{
    moviePosters(tmdbId);
}

/**
 * \brief Load concert backdrops
 */
void TMDbImages::concertBackdrops(TmdbId tmdbId)
{
    movieBackdrops(tmdbId);
}

/**
 * \brief Called when the movie images are downloaded
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
 * \brief Load movie logos
 * \param tmdbId The Movie DB id
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
 * \brief Load movie clear arts
 * \param tmdbId The Movie DB id
 */
void TMDbImages::movieClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load movie cd arts
 * \param tmdbId The Movie DB id
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
 * \brief Load concert logos
 * \param tmdbId The Movie DB id
 */
void TMDbImages::concertLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load concert clear arts
 * \param tmdbId The Movie DB id
 */
void TMDbImages::concertClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load concert cd arts
 * \param tmdbId The Movie DB id
 */
void TMDbImages::concertCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Searches for a TV show
 * \param searchStr Search term
 * \param limit Number of results, if zero, all results are returned
 */
void TMDbImages::searchTvShow(QString searchStr, mediaelch::Locale locale, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
    Q_UNUSED(locale);
}

void TMDbImages::tvShowImages(TvShow* show, TvDbId tvdbId, QVector<ImageType> types, const mediaelch::Locale& locale)
{
    Q_UNUSED(show);
    Q_UNUSED(tvdbId);
    Q_UNUSED(types);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show posters
 * \param tvdbId The TV DB id
 */
void TMDbImages::tvShowPosters(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show backdrops
 * \param tvdbId The TV DB id
 */
void TMDbImages::tvShowBackdrops(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show logos
 * \param tvdbId The TV DB id
 */
void TMDbImages::tvShowLogos(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void TMDbImages::tvShowThumbs(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show clear arts
 * \param tvdbId The TV DB id
 */
void TMDbImages::tvShowClearArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show character arts
 * \param tvdbId The TV DB id
 */
void TMDbImages::tvShowCharacterArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show banners
 * \param tvdbId The TV DB id
 */
void TMDbImages::tvShowBanners(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show thumbs
 * \param tvdbId The TV DB id
 * \param season Season number
 * \param episode Episode number
 */
void TMDbImages::tvShowEpisodeThumb(TvDbId tvdbId,
    SeasonNumber season,
    EpisodeNumber episode,
    const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show season
 * \param tvdbId The TV DB id
 * \param season Season number
 */
void TMDbImages::tvShowSeason(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void TMDbImages::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void TMDbImages::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void TMDbImages::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

bool TMDbImages::hasSettings() const
{
    return false;
}

void TMDbImages::saveSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
}

void TMDbImages::loadSettings(ScraperSettings& settings)
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

} // namespace scraper
} // namespace mediaelch
