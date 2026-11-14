#include "test/test_helpers.h"

#include "utils/Meta.h"

#include <cstdint>
#include <limits>

TEST_CASE("safe_int_cast checks integer ranges", "[utils]")
{
    SECTION("Values within range")
    {
        CHECK(safe_int_cast<std::int32_t>(std::int16_t{-123}) == -123);
        CHECK(safe_int_cast<std::uint32_t>(std::uint16_t{123}) == 123);
        CHECK(safe_int_cast<std::int32_t>(std::uint32_t{123}) == 123);
        CHECK(safe_int_cast<std::uint32_t>(std::int32_t{123}) == 123);
        CHECK(safe_int_cast<std::int8_t>(std::int16_t{127}) == 127);
        CHECK(safe_int_cast<std::int8_t>(std::int16_t{-128}) == -128);
    }
}
