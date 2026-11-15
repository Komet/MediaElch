#include "test/test_helpers.h"

#include "utils/Optional.h"

using mediaelch::Optional;

TEST_CASE("Optional value state and fallback", "[utils]")
{
    SECTION("Empty value uses fallback")
    {
        Optional<int> empty;
        CHECK_FALSE(empty.hasValue());
        CHECK(empty.getOrValue(7) == 7);
    }

    SECTION("Present values are used")
    {
        Optional<int> zero{0};
        CHECK(zero.hasValue());
        CHECK(zero.getOrValue(7) == 0);

        Optional<QString> emptyString{QString{}};
        CHECK(emptyString.hasValue());
        CHECK(emptyString.getOrValue("fallback").isEmpty());
    }
}
