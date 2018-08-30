#include "test/qtCatchHelper.h"
#include "thirdParty/catch2/catch.hpp"

#include "scrapers/VideoBuster.h"

TEST_CASE("VideoBuster returns valid search results", "[scraper][VideoBuster][search][requiresInternet]")
{
    VideoBuster VideoBuster;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(VideoBuster, "Findet Dorie");
        REQUIRE(scraperResults.length() >= 1);
        REQUIRE(scraperResults[0].name == "Findet Dorie");
    }
}
