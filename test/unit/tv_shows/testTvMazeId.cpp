#include "test/test_helpers.h"

#include "tv_shows/TvMazeId.h"

#include <sstream>

TEST_CASE("TvMazeId data type", "[data]")
{
    SECTION("Default Case")
    {
        CHECK(TvMazeId() == TvMazeId(""));

        CHECK(TvMazeId("") == TvMazeId::NoId);
        CHECK(TvMazeId() == TvMazeId::NoId);
    }

    SECTION("Correct TVmaze format")
    {
        CHECK_FALSE(TvMazeId().isValid());
        CHECK_FALSE(TvMazeId("").isValid());
        CHECK_FALSE(TvMazeId(0).isValid());
        CHECK_FALSE(TvMazeId(-1).isValid());

        CHECK(TvMazeId("262504").isValid());
        CHECK(TvMazeId(262504).isValid());
    }

    SECTION("Conversion") { CHECK(TvMazeId("262504").toString() == "262504"); }

    SECTION("output stream")
    {
        std::stringstream stream;

        stream << TvMazeId("262504") << ';';
        stream << TvMazeId() << ';';

        CHECK(stream.str() == "262504;;");
    }

    SECTION("QDebug output")
    {
        QString buffer;
        QDebug stream(&buffer);

        stream << TvMazeId("262504");
        stream << TvMazeId();

        CHECK(buffer == "TvMazeId(\"262504\") TvMazeId(\"\") ");
    }
}
