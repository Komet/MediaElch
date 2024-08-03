#include "globals/ScraperManager.h"

#include "log/Log.h"
#include "scrapers/ScraperConfiguration.h"
#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/concert/tmdb/TmdbConcert.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpire.h"
#include "scrapers/movie/aebn/AEBN.h"
#include "scrapers/movie/aebn/AebnConfiguration.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "scrapers/movie/hotmovies/HotMovies.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovieConfiguration.h"
#include "scrapers/movie/videobuster/VideoBuster.h"
#include "scrapers/music/MusicScraper.h"
#include "scrapers/music/UniversalMusicScraper.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/custom/CustomTvScraper.h"
#include "scrapers/tv_show/fernsehserien_de/FernsehserienDe.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "settings/Settings.h"

namespace mediaelch {

ScraperManager::ScraperManager(Settings& settings, QObject* parent) : QObject(parent), m_settings{settings}
{
    initMovieScrapers();
    initTvScrapers();
    initConcertScrapers();
    initMusicScrapers();
}

ScraperManager::~ScraperManager()
{
    qDeleteAll(m_configurations);
}

/**
 * \brief Returns a list of all movie scrapers
 * \return List of pointers of movie scrapers
 */
const QVector<mediaelch::scraper::MovieScraper*>& ScraperManager::movieScrapers()
{
    return m_movieScrapers;
}

mediaelch::scraper::MovieScraper* ScraperManager::movieScraper(const QString& identifier)
{
    for (auto* scraper : asConst(m_movieScrapers)) {
        if (scraper->meta().identifier == identifier) {
            return scraper;
        }
    }
    MediaElch_Debug_Assert(identifier != "");
    if (identifier != "images.fanarttv") { // TODO
        qCDebug(generic) << "[ScraperManager] No scraper with ID:" << identifier;
    }
    return nullptr;
}

scraper::ConcertScraper* ScraperManager::concertScraper(const QString& identifier)
{
    for (auto* scraper : asConst(m_concertScrapers)) {
        if (scraper->meta().identifier == identifier) {
            return scraper;
        }
    }

    return nullptr;
}

/**
 * \brief Returns a list of all tv scrapers
 * \return List of pointers of tv scrapers
 */
const QVector<mediaelch::scraper::TvScraper*>& ScraperManager::tvScrapers()
{
    return m_tvScrapers;
}

mediaelch::scraper::TvScraper* ScraperManager::tvScraper(const QString& identifier)
{
    for (auto* scraper : asConst(m_tvScrapers)) {
        if (scraper->meta().identifier == identifier) {
            return scraper;
        }
    }
    return nullptr;
}

/**
 * \brief Returns a list of all concert scrapers
 * \return List of pointers of concert scrapers
 */
const QVector<mediaelch::scraper::ConcertScraper*>& ScraperManager::concertScrapers()
{
    return m_concertScrapers;
}

const QVector<mediaelch::scraper::MusicScraper*>& ScraperManager::musicScrapers()
{
    return m_musicScrapers;
}

void ScraperManager::initMovieScrapers()
{
    using namespace mediaelch::scraper;

    auto* tmdbConfig = new TmdbMovieConfiguration(m_settings);
    auto* aebnConfig = new AebnConfiguration(m_settings);
    m_configurations.append(tmdbConfig);

    for (auto* config : m_configurations) {
        config->init();
    }

    m_movieScrapers.append(new TmdbMovie(*tmdbConfig, this));
    m_movieScrapers.append(new ImdbMovie(this));
    m_movieScrapers.append(new VideoBuster(this));
    // Adult Movie Scrapers
    m_movieScrapers.append(new AEBN(*aebnConfig, this));
    m_movieScrapers.append(new HotMovies(this));
    m_movieScrapers.append(new AdultDvdEmpire(this));

    m_movieScrapers.append(CustomMovieScraper::instance(this));
}

void ScraperManager::initTvScrapers()
{
    using namespace mediaelch;

    auto* tmdbTv = new scraper::TmdbTv(this);
    auto* theTvDb = new scraper::TheTvDb(this);
    auto* imdbTv = new scraper::ImdbTv(this);
    auto* tvMaze = new scraper::TvMaze(this);
    auto* fernsehserienDe = new scraper::FernsehserienDe(this);

    m_tvScrapers << tmdbTv << theTvDb << imdbTv << tvMaze << fernsehserienDe;

    for (scraper::TvScraper* scraper : asConst(m_tvScrapers)) {
        qCInfo(generic) << "[TvScraper] Initializing" << scraper->meta().name;
        connect(scraper, &scraper::TvScraper::initialized, this, [](bool wasSuccessful, scraper::TvScraper* tv) {
            if (wasSuccessful) {
                qCInfo(generic) << "[TvScraper] Initialized:" << tv->meta().name;
            } else {
                qCWarning(generic) << "[TvScraper] Initialization failed:" << tv->meta().name;
            }
        });
        scraper->initialize();
    }

    // Only add the Custom TV scraper after the previous ones were added
    // since the constructor explicitly requires them.
    // TODO: Use detail->scraper maps
    scraper::CustomTvScraperConfig config(*tmdbTv, *imdbTv, {}, {});
    m_tvScrapers.append(new scraper::CustomTvScraper(config, this));
}

void ScraperManager::initConcertScrapers()
{
    m_concertScrapers.append(new mediaelch::scraper::TmdbConcert(this));
}

void ScraperManager::initMusicScrapers()
{
    m_musicScrapers.append(new mediaelch::scraper::UniversalMusicScraper(this));
}

} // namespace mediaelch
