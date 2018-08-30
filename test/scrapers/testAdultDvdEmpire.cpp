#include "test/qtCatchHelper.h"
#include "thirdParty/catch2/catch.hpp"

#include "scrapers/AdultDvdEmpire.h"

TEST_CASE("AdultDvdEmpire returns valid search results", "[scraper][AdultDvdEmpire][search][requiresInternet]")
{
    AdultDvdEmpire adultDvdEmpire;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(adultDvdEmpire, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() >= 1);
        REQUIRE(scraperResults[0].name == "Magic Mike XXXL");
    }
}
