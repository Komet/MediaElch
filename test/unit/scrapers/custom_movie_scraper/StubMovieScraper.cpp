#include "test/unit/scrapers/custom_movie_scraper/StubMovieScraper.h"

#include "scrapers/movie/MovieMerger.h"

namespace test {

StubMovieScraper::StubMovieScraper(const QString& id, QObject* parent) : MovieScraper(parent)
{
    m_meta.identifier = id;
    m_meta.name = id;
    m_meta.description = "Stub";
    m_meta.supportedDetails = mediaelch::scraper::allMovieScraperInfos();
    m_meta.supportedLanguages = {"en"};
    m_meta.defaultLocale = "en";
    m_meta.isAdult = false;
}

mediaelch::scraper::MovieSearchJob* StubMovieScraper::search(mediaelch::scraper::MovieSearchJob::Config config)
{
    using namespace mediaelch::scraper;
    return new StubMovieSearchJob(std::move(config), this);
}

mediaelch::scraper::MovieScrapeJob* StubMovieScraper::loadMovie(mediaelch::scraper::MovieScrapeJob::Config config)
{
    using namespace mediaelch::scraper;
    return new StubMovieScrapeJob(std::move(config), stub_movie, this);
}

StubMovieScrapeJob::StubMovieScrapeJob(Config config, Movie& stubMovie, QObject* parent) :
    mediaelch::scraper::MovieScrapeJob(std::move(config), parent)
{
    mediaelch::scraper::copyDetailsToMovie(
        movie(), stubMovie, mediaelch::scraper::allMovieScraperInfos(), false, false);
    setAutoDelete(false);
}

void StubMovieScrapeJob::doStart()
{
    emitFinished();
}

void StubMovieSearchJob::doStart()
{
    emitFinished();
}


} // namespace test
