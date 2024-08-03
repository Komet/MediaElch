#include "scrapers/image/TmdbImages.h"

#include "scrapers/movie/MovieMerger.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "settings/Settings.h"

namespace mediaelch {
namespace scraper {

const QString TmdbImages::ID = "images.tmdb";

TmdbImages::TmdbImages(QObject* parent) : ImageProvider(parent)
{
    m_meta.identifier = ID;
    m_meta.name = "TMDB Images";
    m_meta.description = tr("The Movie Database (TMDB) is a community built movie and TV database. "
                            "Every piece of data has been added by our amazing community dating back to 2008. "
                            "TMDB's strong international focus and breadth of data is largely unmatched and "
                            "something we're incredibly proud of. Put simply, we live and breathe community "
                            "and that's precisely what makes us different.");
    m_meta.website = "https://www.themoviedb.org/tv";
    m_meta.termsOfService = "https://www.themoviedb.org/terms-of-use";
    m_meta.privacyPolicy = "https://www.themoviedb.org/privacy-policy";
    m_meta.help = "https://www.themoviedb.org/talk";
    m_meta.supportedImageTypes = {  //
        ImageType::MovieBackdrop,   //
        ImageType::MoviePoster,     //
        ImageType::ConcertBackdrop, //
        ImageType::ConcertPoster};
    // For officially supported languages, see:
    // https://developers.themoviedb.org/3/configuration/get-primary-translations
    m_meta.supportedLanguages = TmdbMovieConfiguration::supportedLanguages();
    m_meta.defaultLocale = TmdbMovieConfiguration::defaultLocale();

    m_searchResultLimit = 0;

    m_tmdbConfig = new mediaelch::scraper::TmdbMovieConfiguration(*Settings::instance(), this);
    m_tmdb = new mediaelch::scraper::TmdbMovie(*m_tmdbConfig, this);
}

TmdbImages::~TmdbImages() = default;

const ImageProvider::ScraperMeta& TmdbImages::meta() const
{
    return m_meta;
}

/**
 * \brief Searches for a movie
 * \param searchStr The Movie name/search string
 * \param limit Number of results, if zero, all results are returned
 * \see TmdbImages::onSearchMovieFinished
 */
void TmdbImages::searchMovie(QString searchStr, int limit)
{
    using namespace mediaelch::scraper;
    m_searchResultLimit = limit;

    MovieSearchJob::Config config;
    config.locale = m_tmdbConfig->language();
    config.includeAdult = Settings::instance()->showAdultScrapers();
    config.query = searchStr.trimmed();
    auto* searchJob = m_tmdb->search(config);

    connect(searchJob, &MovieSearchJob::searchFinished, this, &TmdbImages::onSearchMovieFinished);
    searchJob->start();
}

void TmdbImages::searchConcert(QString searchStr, int limit)
{
    searchMovie(searchStr, limit);
}

void TmdbImages::onSearchMovieFinished(mediaelch::scraper::MovieSearchJob* searchJob)
{
    auto dls = makeDeleteLaterScope(searchJob);

    QVector<ScraperSearchResult> results;
    if (m_searchResultLimit == 0) {
        results = toOldScraperSearchResult(searchJob->results());

    } else {
        results = toOldScraperSearchResult(searchJob->results().mid(0, m_searchResultLimit));
    }

    emit sigSearchDone(results, searchJob->scraperError());
}

void TmdbImages::moviePosters(TmdbId tmdbId)
{
    using namespace mediaelch::scraper;

    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(tmdbId);
    config.details = {MovieScraperInfo::Poster};
    config.locale = m_tmdb->meta().defaultLocale;

    auto* scrapeJob = m_tmdb->loadMovie(config);
    connect(scrapeJob, &MovieScrapeJob::loadFinished, this, &TmdbImages::onMovieLoadImagesFinished);
    scrapeJob->start();
}

void TmdbImages::movieBackdrops(TmdbId tmdbId)
{
    using namespace mediaelch::scraper;

    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(tmdbId);
    config.details = {MovieScraperInfo::Backdrop};
    config.locale = m_tmdb->meta().defaultLocale;

    auto* scrapeJob = m_tmdb->loadMovie(config);
    connect(scrapeJob, &MovieScrapeJob::loadFinished, this, &TmdbImages::onMovieLoadImagesFinished);
    scrapeJob->start();
}

void TmdbImages::concertPosters(TmdbId tmdbId)
{
    moviePosters(tmdbId);
}

void TmdbImages::concertBackdrops(TmdbId tmdbId)
{
    movieBackdrops(tmdbId);
}

/**
 * \brief Called when the movie images are downloaded
 */
void TmdbImages::onMovieLoadImagesFinished(mediaelch::scraper::MovieScrapeJob* job)
{
    auto dls = makeDeleteLaterScope(job);

    QVector<Poster> posters;
    const QSet<MovieScraperInfo>& details = job->config().details;
    if (details.contains(MovieScraperInfo::Backdrop)) {
        posters = job->movie().images().backdrops();

    } else if (details.contains(MovieScraperInfo::Poster)) {
        posters = job->movie().images().posters();
    }

    emit sigImagesLoaded(posters, {});
}

void TmdbImages::movieImages(Movie* movie, TmdbId tmdbId, QSet<ImageType> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

/**
 * \brief Load movie logos
 * \param tmdbId The Movie DB id
 */
void TmdbImages::movieLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TmdbImages::movieBanners(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TmdbImages::movieThumbs(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load movie clear arts
 * \param tmdbId The Movie DB id
 */
void TmdbImages::movieClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load movie cd arts
 * \param tmdbId The Movie DB id
 */
void TmdbImages::movieCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

void TmdbImages::concertImages(Concert* concert, TmdbId tmdbId, QSet<ImageType> types)
{
    Q_UNUSED(concert);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

/**
 * \brief Load concert logos
 * \param tmdbId The Movie DB id
 */
void TmdbImages::concertLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load concert clear arts
 * \param tmdbId The Movie DB id
 */
void TmdbImages::concertClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load concert cd arts
 * \param tmdbId The Movie DB id
 */
void TmdbImages::concertCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Searches for a TV show
 * \param searchStr Search term
 * \param limit Number of results, if zero, all results are returned
 */
void TmdbImages::searchTvShow(QString searchStr, mediaelch::Locale locale, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
    Q_UNUSED(locale);
}

void TmdbImages::tvShowImages(TvShow* show, TvDbId tvdbId, QSet<ImageType> types, const mediaelch::Locale& locale)
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
void TmdbImages::tvShowPosters(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show backdrops
 * \param tvdbId The TV DB id
 */
void TmdbImages::tvShowBackdrops(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show logos
 * \param tvdbId The TV DB id
 */
void TmdbImages::tvShowLogos(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void TmdbImages::tvShowThumbs(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show clear arts
 * \param tvdbId The TV DB id
 */
void TmdbImages::tvShowClearArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show character arts
 * \param tvdbId The TV DB id
 */
void TmdbImages::tvShowCharacterArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show banners
 * \param tvdbId The TV DB id
 */
void TmdbImages::tvShowBanners(TvDbId tvdbId, const mediaelch::Locale& locale)
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
void TmdbImages::tvShowEpisodeThumb(TvDbId tvdbId,
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
void TmdbImages::tvShowSeason(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void TmdbImages::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void TmdbImages::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void TmdbImages::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

void TmdbImages::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(artistName);
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void TmdbImages::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void TmdbImages::artistFanarts(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void TmdbImages::artistLogos(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void TmdbImages::artistThumbs(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void TmdbImages::albumCdArts(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void TmdbImages::albumThumbs(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

void TmdbImages::artistImages(Artist* artist, MusicBrainzId mbId, QSet<ImageType> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TmdbImages::albumImages(Album* album, MusicBrainzId mbId, QSet<ImageType> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void TmdbImages::albumBooklets(MusicBrainzId mbId)
{
    Q_UNUSED(mbId);
}

} // namespace scraper
} // namespace mediaelch
