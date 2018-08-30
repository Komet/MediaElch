#include "test/qtCatchHelper.h"
#include "thirdParty/catch2/catch.hpp"

#include "scrapers/IMDB.h"

TEST_CASE("IMDb returns valid search results", "[scraper][IMDb][search][requiresInternet]")
{
    IMDB imdb;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(imdb, "Finding Dory");
        REQUIRE(scraperResults.length() >= 2);
        REQUIRE(scraperResults[0].name == "Finding Dory");
        REQUIRE(scraperResults[1].name == "Finding Glory");
    }
}
