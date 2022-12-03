#include "test/test_helpers.h"

#include "data/ImdbId.h"

#include <sstream>

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

    SECTION("Conversion")
    {
        CHECK(ImdbId("tt1234567").toString() == "tt1234567");
    }

    SECTION("output stream")
    {
        std::stringstream stream;

        stream << ImdbId("tt1234567") << ';';
        stream << ImdbId() << ';';

        CHECK(stream.str() == "tt1234567;;");
    }

    SECTION("QDebug output")
    {
        QString buffer;
        QDebug stream(&buffer);

        stream << ImdbId("tt1234567");
        stream << ImdbId();

        CHECK(buffer == "ImdbId(\"tt1234567\") ImdbId(\"\") ");
    }
}
