#include "test/test_helpers.h"

#include "scrapers/movie/omdb/OmdbMovie.h"
#include "scrapers/movie/omdb/OmdbMovieConfiguration.h"
#include "scrapers/movie/omdb/OmdbMovieScrapeJob.h"
#include "scrapers/movie/omdb/OmdbMovieSearchJob.h"
#include "settings/Settings.h"
#include "test/helpers/scraper_helpers.h"
#include "test/mocks/settings/SettingsMock.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static OmdbApi& getOmdbApi()
{
    static auto api = std::make_unique<OmdbApi>();
    if (!api->isInitialized()) {
        QString apiKey = QString::fromLocal8Bit(qgetenv("MEDIAELCH_OMDB_API_KEY"));
        api->setApiKey(apiKey);
        api->initialize();
    }
    return *api;
}

static bool hasOmdbApiKey()
{
    return !qgetenv("MEDIAELCH_OMDB_API_KEY").isEmpty();
}

static MovieScrapeJob::Config makeOmdbConfig(const QString& id)
{
    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(id);
    config.details = allMovieScraperInfos();
    config.locale = mediaelch::Locale::English;
    return config;
}

static auto makeScrapeJob(const QString& id)
{
    return std::make_unique<OmdbMovieScrapeJob>(getOmdbApi(), makeOmdbConfig(id));
}

TEST_CASE("OmdbMovie returns valid search results", "[movie][OMDb][OmdbMovie][search]")
{
    if (!hasOmdbApiKey()) {
        WARN("MEDIAELCH_OMDB_API_KEY not set — skipping OMDb tests");
        return;
    }

    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Finding Dory", mediaelch::Locale::English};
        auto* searchJob = new OmdbMovieSearchJob(getOmdbApi(), config);
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Finding Dory");
    }

    SECTION("Search by IMDB ID returns single correct result")
    {
        MovieSearchJob::Config config{"tt2277860", mediaelch::Locale::English};
        auto* searchJob = new OmdbMovieSearchJob(getOmdbApi(), config);
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() == 1);
        CHECK(scraperResults[0].title == "Finding Dory");
        CHECK(scraperResults[0].identifier.str() == "tt2277860");
    }

    SECTION("Search with year filter returns filtered results")
    {
        MovieSearchJob::Config config{"Inception 2010", mediaelch::Locale::English};
        auto* searchJob = new OmdbMovieSearchJob(getOmdbApi(), config);
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Inception");
        CHECK(scraperResults[0].identifier.str() == "tt1375666");
    }
}

TEST_CASE("OmdbMovie scrapes correct movie details", "[movie][OMDb][OmdbMovie][load_data]")
{
    if (!hasOmdbApiKey()) {
        WARN("MEDIAELCH_OMDB_API_KEY not set — skipping OMDb tests");
        return;
    }

    SECTION("Movie loaded by IMDB ID has all expected fields")
    {
        // Finding Dory (2016)
        auto scrapeJob = makeScrapeJob("tt2277860");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        CHECK(m.imdbId() == ImdbId("tt2277860"));
        CHECK(m.title() == "Finding Dory");
        CHECK(m.released().year() == 2016);
        CHECK(m.runtime() > 0min);
        CHECK(m.certification().toString() == "PG");
        CHECK_THAT(m.overview(), Contains("Dory"));
        CHECK(m.director() == "Andrew Stanton, Angus MacLane");
        CHECK_THAT(m.writer(), Contains("Andrew Stanton"));
        CHECK(m.genres().size() >= 1);
        CHECK(m.countries().size() >= 1);

        // Ratings: OMDb provides IMDB, RT, and Metacritic
        CHECK(m.ratings().hasSource("imdb"));
    }
}
