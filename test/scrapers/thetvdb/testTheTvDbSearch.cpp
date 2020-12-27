#include "test/test_helpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "src/scrapers/tv_show/thetvdb/TheTvDbShowSearchJob.h"
#include "test/scrapers/testScraperHelpers.h"
#include "test/scrapers/thetvdb/testTheTvDbHelper.h"

using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("TheTvDb returns valid search results", "[tv][TheTvDb][search]")
{
    waitForTheTvDbInitialized();

    SECTION("Search by TV show name returns correct results")
    {
        ShowSearchJob::Config config{"The Simpsons", Locale("en-US")};
        auto* searchJob = new TheTvDbShowSearchJob(getTheTvDbApi(), config);
        const auto scraperResults = searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 10);
        CHECK(scraperResults[0].title == "The Simpsons");
        CHECK(scraperResults[0].identifier.str() == "71663");
        CHECK(scraperResults[0].released == QDate(1987, 4, 19));
    }

    SECTION("Search by TV show name in other languages returns correct results")
    {
        ShowSearchJob::Config config{"Scrubs", Locale("pl-PL")};
        auto* searchJob = new TheTvDbShowSearchJob(getTheTvDbApi(), config);
        const auto scraperResults = searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Hoży doktorzy");
        CHECK(scraperResults[0].identifier.str() == "76156");
        CHECK(scraperResults[0].released == QDate(2001, 10, 2));
    }

    SECTION("Search by TV show name returns 0 results for unknown shows")
    {
        ShowSearchJob::Config config{"SomethingThatDoesNotExist", Locale("en-US")};
        auto* searchJob = new TheTvDbShowSearchJob(getTheTvDbApi(), config);
        const auto p = searchTvScraperSync(searchJob);

        CHECK(p.first.length() == 0);
        CHECK(p.second.error == ScraperError::Type::NoError);
    }
}
