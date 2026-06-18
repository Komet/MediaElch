#include "test/test_helpers.h"

#include "data/tv_show/EpisodeNumber.h"

TEST_CASE("EpisodeNumber", "[data][tv_show]")
{
    SECTION("default construction has toInt() == -1, not NoEpisode (-2)")
    {
        CHECK(EpisodeNumber().toInt() == -1);
        CHECK(EpisodeNumber() != EpisodeNumber::NoEpisode);
    }

    SECTION("episode 0 is valid and distinct from NoEpisode")
    {
        CHECK(EpisodeNumber(0).toInt() == 0);
        CHECK(EpisodeNumber(0) != EpisodeNumber::NoEpisode);
    }

    SECTION("positive episode numbers")
    {
        CHECK(EpisodeNumber(1).toInt() == 1);
        CHECK(EpisodeNumber(12).toInt() == 12);
    }

    SECTION("comparison operators")
    {
        CHECK(EpisodeNumber(1) < EpisodeNumber(2));
        CHECK(EpisodeNumber(2) > EpisodeNumber(1));
        CHECK(EpisodeNumber(1) == EpisodeNumber(1));
        CHECK(EpisodeNumber(1) != EpisodeNumber(2));
    }

    SECTION("toPaddedString zero-pads single digits")
    {
        CHECK(EpisodeNumber(1).toPaddedString() == "01");
        CHECK(EpisodeNumber(9).toPaddedString() == "09");
        CHECK(EpisodeNumber(10).toPaddedString() == "10");
        CHECK(EpisodeNumber(99).toPaddedString() == "99");
    }

    SECTION("NoEpisode toPaddedString returns xx")
    {
        CHECK(EpisodeNumber::NoEpisode.toPaddedString() == "xx");
    }

    SECTION("toString returns the number as a string without padding")
    {
        CHECK(EpisodeNumber(5).toString() == "5");
        CHECK(EpisodeNumber(12).toString() == "12");
    }
}
