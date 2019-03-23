#include "test/test_helpers.h"

#include "scrapers/movie/AEBN.h"

TEST_CASE("AEBN returns valid search results", "[scraper][AEBN][search][requires_internet]")
{
    AEBN AEBN;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(AEBN, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Magic Mike XXXL: A Hardcore Parody");
    }
}
