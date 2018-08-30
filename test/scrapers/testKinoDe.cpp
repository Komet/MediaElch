#include "test/qtCatchHelper.h"
#include "thirdParty/catch2/catch.hpp"

#include "scrapers/KinoDe.h"

TEST_CASE("KinoDe returns valid search results", "[scraper][KinoDe][search][requiresInternet]")
{
    KinoDe KinoDe;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(KinoDe, "Findet Dorie");
        REQUIRE(scraperResults.length() >= 1);
        REQUIRE(scraperResults[0].name == "Findet Dorie");
    }
}
