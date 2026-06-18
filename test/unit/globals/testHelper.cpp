#include "test/test_helpers.h"

#include "globals/Helper.h"

using namespace helper;

TEST_CASE("helper::monthNameToInt", "[globals]")
{
    SECTION("all 12 months")
    {
        CHECK(monthNameToInt("Jan") == 1);
        CHECK(monthNameToInt("Feb") == 2);
        CHECK(monthNameToInt("Mar") == 3);
        CHECK(monthNameToInt("Apr") == 4);
        CHECK(monthNameToInt("May") == 5);
        CHECK(monthNameToInt("June") == 6);
        CHECK(monthNameToInt("July") == 7);
        CHECK(monthNameToInt("Aug") == 8);
        CHECK(monthNameToInt("Sept") == 9);
        CHECK(monthNameToInt("Oct") == 10);
        CHECK(monthNameToInt("Nov") == 11);
        CHECK(monthNameToInt("Dec") == 12);
    }

    SECTION("unknown month returns -1")
    {
        CHECK(monthNameToInt("Xyz") == -1);
        CHECK(monthNameToInt("") == -1);
        CHECK(monthNameToInt("January") == -1); // full names not supported
    }
}

TEST_CASE("helper::sanitizeFileName", "[globals]")
{
    SECTION("replaces : with space (consecutive spaces are removed)")
    {
        QString name = "movie: title";
        sanitizeFileName(name);
        CHECK(name == "movie title");
    }

    SECTION("removes ? and * entirely (no space added)")
    {
        QString name = "what? why*";
        sanitizeFileName(name);
        CHECK(name == "what why");
    }

    SECTION("replaces slashes with space")
    {
        QString name = "a/b\\c";
        sanitizeFileName(name);
        CHECK(name == "a b c");
    }

    SECTION("replaces < > | with space")
    {
        QString name = "a<b>c|d";
        sanitizeFileName(name);
        CHECK(name == "a b c d");
    }

    SECTION("removes leading dots (hidden files on Unix)")
    {
        QString name = "...hidden";
        sanitizeFileName(name);
        CHECK(name == "hidden");
    }

    SECTION("collapses consecutive spaces")
    {
        QString name = "a  b   c";
        sanitizeFileName(name);
        CHECK(name == "a b c");
    }
}

TEST_CASE("helper::isDvd", "[globals]")
{
    SECTION("path ending in VIDEO_TS.IFO with VIDEO_TS parent is a DVD")
    {
        CHECK(isDvd("/movies/MyMovie/VIDEO_TS/VIDEO_TS.IFO") == true);
    }

    SECTION("plain file path is not a DVD")
    {
        CHECK(isDvd("/movies/movie.mkv") == false);
    }
}

TEST_CASE("helper::isBluRay", "[globals]")
{
    SECTION("path ending in BDMV/index.bdmv is a BluRay")
    {
        CHECK(isBluRay("/movies/MyMovie/BDMV/index.bdmv") == true);
    }

    SECTION("plain file path is not a BluRay")
    {
        CHECK(isBluRay("/movies/movie.mkv") == false);
    }
}
