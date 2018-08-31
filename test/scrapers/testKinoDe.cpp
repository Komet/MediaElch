#include "test/test_helpers.h"

#include "scrapers/KinoDe.h"

TEST_CASE("KinoDe returns valid search results", "[scraper][KinoDe][search][requires_internet]")
{
    KinoDe KinoDe;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(KinoDe, "Findet Dorie");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Findet Dorie");
    }
}
