#include "test/test_helpers.h"

#include "globals/Time.h"

using namespace mediaelch;

TEST_CASE("secondsToTimeCode", "[time]")
{
    const auto sec = [](unsigned seconds) { return seconds; };
    const auto min = [](unsigned minutes) { return minutes * 60; };
    const auto h = [](unsigned hours) { return hours * 60 * 60; };
    const auto d = [](unsigned days) { return days * 60 * 60 * 24; };

    SECTION("only seconds/minutes/hours/days")
    {
        CHECK(secondsToTimeCode(0) == "00:00");
        CHECK(secondsToTimeCode(sec(7)) == "00:07");
        CHECK(secondsToTimeCode(min(7)) == "07:00");
        CHECK(secondsToTimeCode(h(7)) == "07:00:00");
        CHECK(secondsToTimeCode(d(22)) == "22d00:00:00");
    }

    SECTION("combinations")
    {
        CHECK(secondsToTimeCode(min(11) + sec(3)) == "11:03");
        CHECK(secondsToTimeCode(min(7) + sec(53)) == "07:53");
        CHECK(secondsToTimeCode(h(5) + min(7) + sec(53)) == "05:07:53");
        CHECK(secondsToTimeCode(d(1) + h(11) + min(17) + sec(3)) == "1d11:17:03");
    }
}
