#include "test/test_helpers.h"

#include "tv_shows/TvDbId.h"

TEST_CASE("TvDbId data type", "[data]")
{
    SECTION("Default Case")
    {
        // only valid IDs are comparible
        CHECK(TvDbId() != TvDbId(""));

        CHECK_FALSE(TvDbId() == TvDbId(""));
        CHECK_FALSE(TvDbId().isValid());

        CHECK(TvDbId("") != TvDbId::NoId);
        CHECK(TvDbId() != TvDbId::NoId);
    }
    SECTION("Correct TheTvDb format")
    {
        CHECK_FALSE(TvDbId(0).isValid());
        CHECK_FALSE(TvDbId("id0").isValid());
        CHECK_FALSE(TvDbId("0").isValid());
        // \todo Currently only checks whether the id isn't empty.
        CHECK(TvDbId("id1234567").isValid());
        CHECK(TvDbId("1234567").isValid());
    }
    SECTION("Conversion")
    {
        CHECK(TvDbId(1234567).toString() == "1234567");
        CHECK(TvDbId(1234567).withPrefix() == "id1234567");
        CHECK(TvDbId("id1234567").toString() == "1234567");
        CHECK(TvDbId("id1234567").withPrefix() == "id1234567");
    }
}
