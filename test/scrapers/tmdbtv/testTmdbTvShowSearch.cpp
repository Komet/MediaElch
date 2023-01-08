#include "test/test_helpers.h"

#include "src/scrapers/tv_show/tmdb/TmdbTvShowSearchJob.h"
#include "test/scrapers/testScraperHelpers.h"
#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"

using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("TmdbTv returns valid search results", "[tv][TmdbTv][search]")
{
    SECTION("Search by TV show name returns correct results")
    {
        ShowSearchJob::Config config{"Simpsons", Locale::English};
        auto* searchJob = new TmdbTvShowSearchJob(getTmdbApi(), config);
        const auto scraperResults = searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].title == "The Simpsons");
        CHECK(scraperResults[0].identifier.str() == "456");
        CHECK(scraperResults[0].released == QDate(1989, 12, 17));
    }

    SECTION("Search by TV show name in other languages returns correct results")
    {
        ShowSearchJob::Config config{"Scrubs", Locale("de-DE")};
        auto* searchJob = new TmdbTvShowSearchJob(getTmdbApi(), config);
        const auto scraperResults = searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 3);
        CHECK(scraperResults[0].title == "Scrubs - Die Anfänger");
        CHECK(scraperResults[0].identifier.str() == "4556");
        CHECK(scraperResults[0].released == QDate(2001, 10, 2));
    }

    SECTION("Search by TV show name returns 0 results for unknown shows")
    {
        ShowSearchJob::Config config{"SomethingThatDoesNotExist", Locale::English};
        auto* searchJob = new TmdbTvShowSearchJob(getTmdbApi(), config);
        const auto p = searchTvScraperSync(searchJob, true);

        CHECK(p.first.length() == 0);
        CHECK(p.second.error == ScraperError::Type::NoError);
    }

    SECTION("Search by TV show name returns correct results for number-only title")
    {
        ShowSearchJob::Config config{"1899", Locale::English};
        auto* searchJob = new TmdbTvShowSearchJob(getTmdbApi(), config);
        const auto scraperResults = searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "1899");
        CHECK(scraperResults[0].identifier.str() == "90669");
        CHECK(scraperResults[0].released == QDate(2022, 11, 17));
    }
}
