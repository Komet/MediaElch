#include "test/qtCatchHelper.h"
#include "thirdParty/catch2/catch.hpp"

#include "scrapers/HotMovies.h"

TEST_CASE("HotMovies returns valid search results", "[scraper][HotMovies][search][requiresInternet]")
{
    HotMovies HotMovies;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(HotMovies, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() >= 1);
        REQUIRE(scraperResults[0].name == "Magic Mike XXXL: A Hardcore Parody");
    }
}
