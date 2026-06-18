#include "test/test_helpers.h"

#include "utils/Math.h"

using namespace mediaelch::math;

TEST_CASE("math::clamp", "[utils]")
{
    SECTION("value in range is returned unchanged")
    {
        CHECK(clamp(0, 10, 5) == 5);
        CHECK(clamp(0, 10, 0) == 0);
        CHECK(clamp(0, 10, 10) == 10);
    }

    SECTION("value below min is clamped to min")
    {
        CHECK(clamp(0, 10, -1) == 0);
        CHECK(clamp(5, 10, 0) == 5);
    }

    SECTION("value above max is clamped to max")
    {
        CHECK(clamp(0, 10, 11) == 10);
        CHECK(clamp(0, 5, 100) == 5);
    }

    SECTION("min equals max")
    {
        CHECK(clamp(5, 5, 5) == 5);
        CHECK(clamp(5, 5, 0) == 5);
        CHECK(clamp(5, 5, 9) == 5);
    }

    SECTION("negative ranges")
    {
        CHECK(clamp(-10, -1, -5) == -5);
        CHECK(clamp(-10, -1, -20) == -10);
        CHECK(clamp(-10, -1, 0) == -1);
    }
}
