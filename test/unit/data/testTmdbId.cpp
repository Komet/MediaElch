#include "test/test_helpers.h"

#include "data/TmdbId.h"

#include <sstream>

TEST_CASE("TmdbId data type", "[data][tmdb]")
{
    SECTION("Default Case")
    {
        CHECK(TmdbId() == TmdbId(""));

        CHECK(TmdbId("") == TmdbId::NoId);
        CHECK(TmdbId() == TmdbId::NoId);
    }

    SECTION("Correct TMDb format")
    {
        CHECK_FALSE(TmdbId().isValid());
        CHECK_FALSE(TmdbId("").isValid());
        CHECK_FALSE(TmdbId("0").isValid());
        CHECK_FALSE(TmdbId("tmdb0").isValid());
        CHECK_FALSE(TmdbId("id123").isValid());

        CHECK(TmdbId("tmdb262504").isValid());
        CHECK(TmdbId("262504").isValid());
    }

    SECTION("Conversion")
    {
        CHECK(TmdbId("262504").toString() == "262504");
        CHECK(TmdbId("262504").withPrefix() == "id262504");
    }

    SECTION("output stream")
    {
        std::stringstream stream;

        stream << TmdbId("262504") << ';';
        stream << TmdbId() << ';';

        CHECK(stream.str() == "262504;;");
    }

    SECTION("QDebug output")
    {
        QString buffer;
        QDebug stream(&buffer);

        stream << TmdbId("262504");
        stream << TmdbId();

        CHECK(buffer == "TmdbId(\"262504\") TmdbId(\"\") ");
    }
}
