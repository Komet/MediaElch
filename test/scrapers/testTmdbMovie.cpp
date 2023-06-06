#include "test/test_helpers.h"

#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovieScrapeJob.h"
#include "scrapers/movie/tmdb/TmdbMovieSearchJob.h"
#include "settings/Settings.h"
#include "test/helpers/scraper_helpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static TmdbApi& getTmdbApi()
{
    static auto api = std::make_unique<TmdbApi>();
    if (!api->isInitialized()) {
        QEventLoop loop;
        QEventLoop::connect(api.get(), &TmdbApi::initialized, &loop, &QEventLoop::quit);
        api->initialize();
        loop.exec();
    }
    return *api;
}

static MovieScrapeJob::Config makeTmdbConfig(const QString& id)
{
    static auto tmdb = std::make_unique<TmdbMovie>();
    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(id);
    config.details = tmdb->meta().supportedDetails;
    config.locale = tmdb->meta().defaultLocale;
    return config;
}

static auto makeScrapeJob(QString id)
{
    return std::make_unique<TmdbMovieScrapeJob>(getTmdbApi(), makeTmdbConfig(id));
}

TEST_CASE("TmdbMovie returns valid search results", "[TMDb][TmdbMovie][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Finding Dory", mediaelch::Locale::English};
        auto* searchJob = new TmdbMovieSearchJob(getTmdbApi(), config);
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].title == "Finding Dory");
        CHECK(scraperResults[1].title.length() > 5); // second result changes regularly
    }
}

TEST_CASE("TmdbMovie scrapes correct movie details", "[TMDb][TmdbMovie][load_data]")
{
    SECTION("'Normal' movie loaded by using IMDb id")
    {
        auto scrapeJob = makeScrapeJob("tt2277860");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        REQUIRE(m.tmdbId() == TmdbId("127380"));

        test::scraper::compareAgainstReference(m, "scrapers/tmdb/Finding_Dory_tt2277860");
    }

    SECTION("'Normal' movie loaded by using TmdbMovie id")
    {
        auto scrapeJob = makeScrapeJob("127380");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        REQUIRE(m.tmdbId() == TmdbId("127380"));

        test::scraper::compareAgainstReference(m, "scrapers/tmdb/Finding_Dory_tmdb127380");
    }
}
