#include "test/qtCatchHelper.h"
#include "thirdParty/catch2/catch.hpp"

#include "scrapers/AEBN.h"

TEST_CASE("AEBN returns valid search results", "[scraper][AEBN][search][requiresInternet]")
{
    AEBN AEBN;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(AEBN, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() >= 1);
        REQUIRE(scraperResults[0].name == "Magic Mike XXXL: A Hardcore Parody");
    }
}
