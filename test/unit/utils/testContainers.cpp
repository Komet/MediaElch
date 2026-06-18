#include "test/test_helpers.h"

#include "utils/Containers.h"

using namespace mediaelch;

TEST_CASE("split_string_trimmed", "[utils]")
{
    SECTION("splits on delimiter and trims whitespace")
    {
        const QStringList result = split_string_trimmed("a , b , c", ",");
        REQUIRE(result.size() == 3);
        CHECK(result[0] == "a");
        CHECK(result[1] == "b");
        CHECK(result[2] == "c");
    }

    SECTION("empty string returns empty list")
    {
        CHECK(split_string_trimmed("", ",").isEmpty());
    }

    SECTION("no delimiter in string returns single-element list")
    {
        const QStringList result = split_string_trimmed("hello", ",");
        REQUIRE(result.size() == 1);
        CHECK(result[0] == "hello");
    }

    SECTION("whitespace-only entries are removed")
    {
        const QStringList result = split_string_trimmed("a ,  , b", ",");
        REQUIRE(result.size() == 2);
        CHECK(result[0] == "a");
        CHECK(result[1] == "b");
    }

    SECTION("multi-character delimiter")
    {
        const QStringList result = split_string_trimmed("a :: b :: c", "::");
        REQUIRE(result.size() == 3);
        CHECK(result[0] == "a");
        CHECK(result[1] == "b");
        CHECK(result[2] == "c");
    }
}
