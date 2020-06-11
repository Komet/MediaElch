#include "globals/ScraperManager.h"

#include "scrapers/ScraperInterface.h"
#include "scrapers/concert/ConcertScraperInterface.h"
#include "scrapers/concert/TMDbConcerts.h"
#include "scrapers/movie/AEBN.h"
#include "scrapers/movie/AdultDvdEmpire.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/HotMovies.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/MovieScraperInterface.h"
#include "scrapers/movie/OFDb.h"
#include "scrapers/movie/TMDb.h"
#include "scrapers/movie/VideoBuster.h"
#include "scrapers/music/MusicScraperInterface.h"
#include "scrapers/music/UniversalMusicScraper.h"
#include "scrapers/tv_show/TheTvDb.h"
#include "scrapers/tv_show/TvScraperInterface.h"

namespace mediaelch {

ScraperManager::ScraperManager(QObject* parent) : QObject(parent)
{
    initMovieScrapers();
    initTvScrapers();
    initConcertScrapers();
    initMusicScrapers();
}

/**
 * \brief Returns a list of all movie scrapers
 * \return List of pointers of movie scrapers
 */
const QVector<MovieScraperInterface*>& ScraperManager::movieScrapers()
{
    return m_movieScrapers;
}

MovieScraperInterface* ScraperManager::movieScraper(const QString& identifier)
{
    for (auto* scraper : m_movieScrapers) {
        if (scraper->identifier() == identifier) {
            return scraper;
        }
    }

    return nullptr;
}

/**
 * \brief Returns a list of all tv scrapers
 * \return List of pointers of tv scrapers
 */
const QVector<TvScraperInterface*>& ScraperManager::tvScrapers()
{
    return m_tvScrapers;
}

TvScraperInterface* ScraperManager::tvScraper(const QString& identifier)
{
    for (auto* scraper : m_tvScrapers) {
        if (scraper->identifier() == identifier) {
            return scraper;
        }
    }
    return nullptr;
}

/**
 * \brief Returns a list of all concert scrapers
 * \return List of pointers of concert scrapers
 */
const QVector<ConcertScraperInterface*>& ScraperManager::concertScrapers()
{
    return m_concertScrapers;
}

const QVector<MusicScraperInterface*>& ScraperManager::musicScrapers()
{
    return m_musicScrapers;
}

QVector<MovieScraperInterface*> ScraperManager::constructNativeScrapers(QObject* scraperParent)
{
    QVector<MovieScraperInterface*> scrapers;
    scrapers.append(new TMDb(scraperParent));
    scrapers.append(new IMDB(scraperParent));
    scrapers.append(new OFDb(scraperParent));
    scrapers.append(new VideoBuster(scraperParent));
    return scrapers;
}

void ScraperManager::initMovieScrapers()
{
    m_movieScrapers.append(new TMDb(this));
    m_movieScrapers.append(new IMDB(this));
    m_movieScrapers.append(new OFDb(this));
    m_movieScrapers.append(new VideoBuster(this));
    // Adult Movie Scrapers
    m_movieScrapers.append(new AEBN(this));
    m_movieScrapers.append(new HotMovies(this));
    m_movieScrapers.append(new AdultDvdEmpire(this));

    m_movieScrapers.append(CustomMovieScraper::instance(this));
}

void ScraperManager::initTvScrapers()
{
    m_tvScrapers.append(new TheTvDb(this));
}

void ScraperManager::initConcertScrapers()
{
    m_concertScrapers.append(new TMDbConcerts(this));
}

void ScraperManager::initMusicScrapers()
{
    m_musicScrapers.append(new UniversalMusicScraper(this));
}

} // namespace mediaelch
