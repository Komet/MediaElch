#include "test/test_helpers.h"

#include "src/scrapers/tv_show/tvmaze/TvMazeShowSearchJob.h"
#include "test/scrapers/testScraperHelpers.h"
#include "test/scrapers/tvmaze/testTvMazeHelper.h"

using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("TvMaze returns valid search results", "[tv][TvMaze][search]")
{
    SECTION("Search by TV show name returns correct results")
    {
        ShowSearchJob::Config config{"Simpsons", Locale::English};
        auto* searchJob = new TvMazeShowSearchJob(getTvMazeApi(), config);
        const auto scraperResults = searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].title == "The Simpsons");
        CHECK(scraperResults[0].identifier.str() == "83");
        CHECK(scraperResults[0].released == QDate(1989, 12, 17));
    }

    SECTION("Search by TV show name returns 0 results for unknown shows")
    {
        ShowSearchJob::Config config{"SomethingThatDoesNotExist", Locale::English};
        auto* searchJob = new TvMazeShowSearchJob(getTvMazeApi(), config);
        const auto p = searchTvScraperSync(searchJob, true);

        CHECK(p.first.length() == 0);
        CHECK(p.second.error == ScraperError::Type::NoError);
    }
}
