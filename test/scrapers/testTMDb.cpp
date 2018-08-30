#include "test/qtCatchHelper.h"
#include "thirdParty/catch2/catch.hpp"

#include "scrapers/TMDb.h"

TEST_CASE("TMDb returns valid search results", "[scraper][TMDb][search][requiresInternet]")
{
    TMDb TMDb;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(TMDb, "Finding Dory");
        REQUIRE(scraperResults.length() >= 2);
        REQUIRE(scraperResults[0].name == "Finding Dory");
        REQUIRE(scraperResults[1].name == "Marine Life Interviews");
    }
}
