#include "test/test_helpers.h"

#include "data/ImdbId.h"

TEST_CASE("ImdbId data type", "[data]")
{
    SECTION("Default Case")
    {
        CHECK(ImdbId() == ImdbId(""));
        CHECK(!ImdbId().isValid());

        CHECK(ImdbId("") == ImdbId::NoId);
        CHECK(ImdbId() == ImdbId::NoId);
    }
    SECTION("Correct IMDb format")
    {
        CHECK(ImdbId("tt1234567").isValid());
        CHECK(ImdbId::isValidFormat("tt1234567"));
    }
    SECTION("Conversion") { CHECK(ImdbId("tt1234567").toString() == "tt1234567"); }
}
