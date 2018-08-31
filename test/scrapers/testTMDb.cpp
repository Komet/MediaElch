#include "test/test_helpers.h"

#include "scrapers/TMDb.h"

TEST_CASE("TMDb returns valid search results", "[scraper][TMDb][search][requires_internet]")
{
    TMDb TMDb;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(TMDb, "Finding Dory");
        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].name == "Finding Dory");
        CHECK(scraperResults[1].name == "Marine Life Interviews");
    }
}
