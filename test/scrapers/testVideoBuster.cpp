#include "test/test_helpers.h"

#include "scrapers/VideoBuster.h"

TEST_CASE("VideoBuster returns valid search results", "[scraper][VideoBuster][search][requires_internet]")
{
    VideoBuster VideoBuster;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(VideoBuster, "Findet Dorie");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Findet Dorie");
    }
}
