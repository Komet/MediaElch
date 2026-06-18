#include "test/test_helpers.h"

#include "data/tv_show/SeasonNumber.h"

TEST_CASE("SeasonNumber", "[data][tv_show]")
{
    SECTION("default construction equals NoSeason")
    {
        CHECK(SeasonNumber() == SeasonNumber::NoSeason);
    }

    SECTION("SpecialsSeason is season 0 and not NoSeason")
    {
        CHECK(SeasonNumber::SpecialsSeason.toInt() == 0);
        CHECK(SeasonNumber::SpecialsSeason != SeasonNumber::NoSeason);
    }

    SECTION("positive season numbers")
    {
        CHECK(SeasonNumber(1).toInt() == 1);
        CHECK(SeasonNumber(3).toInt() == 3);
    }

    SECTION("comparison operators")
    {
        CHECK(SeasonNumber(1) < SeasonNumber(2));
        CHECK(SeasonNumber(2) > SeasonNumber(1));
        CHECK(SeasonNumber(1) == SeasonNumber(1));
        CHECK(SeasonNumber(1) != SeasonNumber(2));
    }

    SECTION("toPaddedString zero-pads single digits")
    {
        CHECK(SeasonNumber(1).toPaddedString() == "01");
        CHECK(SeasonNumber(9).toPaddedString() == "09");
        CHECK(SeasonNumber(10).toPaddedString() == "10");
    }

    SECTION("NoSeason toPaddedString returns xx")
    {
        CHECK(SeasonNumber::NoSeason.toPaddedString() == "xx");
    }

    SECTION("toString returns the number as a string")
    {
        CHECK(SeasonNumber(5).toString() == "5");
    }
}
