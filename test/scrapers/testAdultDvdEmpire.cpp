#include "test/test_helpers.h"

#include "scrapers/AdultDvdEmpire.h"

TEST_CASE("AdultDvdEmpire returns valid search results", "[scraper][AdultDvdEmpire][search][requires_internet]")
{
    AdultDvdEmpire adultDvdEmpire;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(adultDvdEmpire, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Magic Mike XXXL");
    }
}
