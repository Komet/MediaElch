#include "TheTvDbImages.h"

#include "globals/Manager.h"
#include "scrapers/tv_show/ShowMerger.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "settings/Settings.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/TvShowEpisode.h"

namespace mediaelch {
namespace scraper {

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
    m_searchResultLimit = 0;
    m_supportedLanguages = {"bg",
        "zh",
        "hr",
        "cs",
        "da",
        "nl",
        "en",
        "fi",
        "fr",
        "de",
        "el",
        "he",
        "hu",
        "it",
        "ja",
        "ko",
        "no",
        "pl",
        "pt",
        "ru",
        "sl",
        "es",
        "sv",
        "tr"};

    connect(m_dummyShow, &TvShow::sigLoaded, this, &TheTvDbImages::onLoadTvShowDataFinished);
    connect(m_dummyEpisode, &TvShowEpisode::sigLoaded, this, &TheTvDbImages::onLoadTvShowDataFinished);
}

/**
 * \brief Returns the name of this image provider
 * \return Name of this image provider
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

mediaelch::Locale TheTvDbImages::defaultLanguage()
{
    return "en";
}

const QVector<mediaelch::Locale>& TheTvDbImages::supportedLanguages()
{
    return m_supportedLanguages;
}

QSet<ImageType> TheTvDbImages::provides()
{
    return m_provides;
}

/**
 * \brief Searches for a movie
 * \param searchStr The Movie name/search string
 * \param limit Number of results, if zero, all results are returned
 */
void TheTvDbImages::searchMovie(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

/**
 * \brief Searches for a concert
 * \param searchStr The Concert name/search string
 * \param limit Number of results, if zero, all results are returned
 */
void TheTvDbImages::searchConcert(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

/**
 * \brief Searches for a TV show
 * \param searchStr The TV show name/search string
 * \param limit Number of results, if zero, all results are returned
 * \see TheTvDbImages::onSearchTvShowFinished
 */
void TheTvDbImages::searchTvShow(QString searchStr, mediaelch::Locale locale, int limit)
{
    using namespace mediaelch::scraper;
    auto* tvdb = dynamic_cast<TheTvDb*>(Manager::instance()->scrapers().tvScraper(TheTvDb::ID));
    if (tvdb == nullptr) {
        qFatal("[FanartTv] Cast to TheTvDb* failed!");
    }

    m_searchResultLimit = limit;
    ShowSearchJob::Config config{searchStr, locale, false};

    auto* searchJob = tvdb->search(config);
    connect(searchJob, &ShowSearchJob::sigFinished, this, &TheTvDbImages::onSearchTvShowFinished, Qt::UniqueConnection);
    searchJob->execute();
}

void TheTvDbImages::onSearchTvShowFinished(mediaelch::scraper::ShowSearchJob* searchJob)
{
    const auto results = toOldScraperSearchResult(searchJob->results());
    const auto error = searchJob->error();
    searchJob->deleteLater();

    if (m_searchResultLimit == 0) {
        emit sigSearchDone(results, error);
    } else {
        emit sigSearchDone(results.mid(0, m_searchResultLimit), error);
    }
}

void TheTvDbImages::loadTvShowData(TvDbId tvdbId, ImageType type, const mediaelch::Locale& locale)
{
    using namespace mediaelch::scraper;
    m_currentType = type;
    m_dummyShow->clear();

    auto* tvdb = dynamic_cast<TheTvDb*>(Manager::instance()->scrapers().tvScraper(TheTvDb::ID));
    if (tvdb == nullptr) {
        qFatal("[FanartTv] Cast to TheTvDb* failed!");
    }

    if (type == ImageType::TvShowEpisodeThumb) {
        EpisodeScrapeJob::Config config(EpisodeIdentifier(tvdbId), locale, {EpisodeScraperInfo::Thumbnail});

        const auto episodeLoaded = [this](EpisodeScrapeJob* job) {
            m_dummyEpisode->clear(job->config().details);
            copyDetailsToEpisode(*m_dummyEpisode, job->episode(), job->config().details);
            job->deleteLater();
        };

        auto* scrapeJob = tvdb->loadEpisode(config);
        connect(scrapeJob, &EpisodeScrapeJob::sigFinished, this, episodeLoaded, Qt::UniqueConnection);
        scrapeJob->execute();

    } else {
        const QSet<ShowScraperInfo> infosToLoad{ShowScraperInfo::Banner,
            ShowScraperInfo::Fanart,
            ShowScraperInfo::Poster,
            ShowScraperInfo::SeasonPoster,
            ShowScraperInfo::SeasonBanner,
            ShowScraperInfo::SeasonBackdrop};
        m_dummyShow->scrapeData(tvdb,
            mediaelch::scraper::ShowIdentifier(tvdbId),
            locale,
            SeasonOrder::Aired,
            TvShowUpdateType::Show,
            infosToLoad,
            {});
    }
}

/**
 * \brief Called when the TV show images are downloaded
 * \see TMDbImages::parseTvShowData
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

void TheTvDbImages::tvShowImages(TvShow* show, TvDbId tvdbId, QVector<ImageType> types, const mediaelch::Locale& locale)
{
    Q_UNUSED(show)
    Q_UNUSED(tvdbId)
    Q_UNUSED(types)
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show posters
 * \param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowPosters(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    loadTvShowData(tvdbId, ImageType::TvShowPoster, locale);
}

/**
 * \brief Load TV show backdrops
 * \param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowBackdrops(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    loadTvShowData(tvdbId, ImageType::TvShowBackdrop, locale);
}

/**
 * \brief Load TV show banners
 * \param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowBanners(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    loadTvShowData(tvdbId, ImageType::TvShowBanner, locale);
}

/**
 * \brief Load TV show thumbs
 * \param tvdbId The TV DB id
 * \param season Season number
 * \param episode Episode number
 */
void TheTvDbImages::tvShowEpisodeThumb(TvDbId tvdbId,
    SeasonNumber season,
    EpisodeNumber episode,
    const mediaelch::Locale& locale)
{
    m_dummyEpisode->clear();
    m_dummyEpisode->setSeason(season);
    m_dummyEpisode->setEpisode(episode);
    loadTvShowData(tvdbId, ImageType::TvShowEpisodeThumb, locale);
}

/**
 * \brief Load TV show season
 * \param tvdbId The TV DB id
 * \param season Season number
 */
void TheTvDbImages::tvShowSeason(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    m_season = season;
    loadTvShowData(tvdbId, ImageType::TvShowSeasonPoster, locale);
}

void TheTvDbImages::tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    m_season = season;
    loadTvShowData(tvdbId, ImageType::TvShowSeasonBanner, locale);
}

// UNSUPPORTED

void TheTvDbImages::movieImages(Movie* movie, TmdbId tmdbId, QVector<ImageType> types)
{
    Q_UNUSED(movie);
    Q_UNUSED(tmdbId);
    Q_UNUSED(types);
}

void TheTvDbImages::tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(locale)
}

/**
 * \brief Would load movie posters (not supported by fanart.tv)
 */
void TheTvDbImages::moviePosters(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load movie backdrops
 */
void TheTvDbImages::movieBackdrops(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load movie logos
 * \param tmdbId The Movie DB id
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
 * \brief Load movie clear arts
 * \param tmdbId The Movie DB id
 */
void TheTvDbImages::movieClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load movie cd arts
 * \param tmdbId The Movie DB id
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
 * \brief Would load concert posters (not supported by fanart.tv)
 */
void TheTvDbImages::concertPosters(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load concert backdrops
 */
void TheTvDbImages::concertBackdrops(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load concert logos
 * \param tmdbId The Movie DB id
 */
void TheTvDbImages::concertLogos(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load concert clear arts
 * \param tmdbId The Movie DB id
 */
void TheTvDbImages::concertClearArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load concert cd arts
 * \param tmdbId The Movie DB id
 */
void TheTvDbImages::concertCdArts(TmdbId tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * \brief Load TV show logos
 * \param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowLogos(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

void TheTvDbImages::tvShowThumbs(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show clear arts
 * \param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowClearArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId)
    Q_UNUSED(locale)
}

/**
 * \brief Load TV show character arts
 * \param tvdbId The TV DB id
 */
void TheTvDbImages::tvShowCharacterArts(TvDbId tvdbId, const mediaelch::Locale& locale)
{
    Q_UNUSED(tvdbId)
    Q_UNUSED(locale)
}

void TheTvDbImages::tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale)
{
    Q_UNUSED(season);
    loadTvShowData(tvdbId, ImageType::TvShowSeasonBackdrop, locale);
}

bool TheTvDbImages::hasSettings() const
{
    return false;
}

void TheTvDbImages::loadSettings(ScraperSettings& settings)
{
    Q_UNUSED(settings);
    // no-op, settings loaded on-demand.
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

} // namespace scraper
} // namespace mediaelch
