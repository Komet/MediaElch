#include "test/test_helpers.h"

#include "scrapers/HotMovies.h"

TEST_CASE("HotMovies returns valid search results", "[scraper][HotMovies][search][requires_internet]")
{
    HotMovies HotMovies;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(HotMovies, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Magic Mike XXXL: A Hardcore Parody");
    }
}
