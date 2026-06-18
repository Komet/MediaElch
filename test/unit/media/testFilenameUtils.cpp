#include "test/test_helpers.h"

#include "media/FilenameUtils.h"

using namespace mediaelch::file;

TEST_CASE("stackedBaseName", "[media]")
{
    SECTION("removes -cd1 style suffix")
    {
        CHECK(stackedBaseName("movie-cd1.avi") == "movie");
        CHECK(stackedBaseName("movie-cd2.avi") == "movie");
        CHECK(stackedBaseName("movie-part1.mkv") == "movie");
    }

    SECTION("no stack suffix: returns original filename unchanged")
    {
        CHECK(stackedBaseName("movie.mkv") == "movie.mkv");
        CHECK(stackedBaseName("The.Dark.Knight.mkv") == "The.Dark.Knight.mkv");
    }

    SECTION("case-insensitive suffix matching")
    {
        CHECK(stackedBaseName("movie-CD1.avi") == "movie");
        CHECK(stackedBaseName("movie-Part1.mkv") == "movie");
    }
}

TEST_CASE("withoutExtension", "[media]")
{
    SECTION("removes extension")
    {
        CHECK(withoutExtension("foo.mkv") == "foo");
        CHECK(withoutExtension("movie.avi") == "movie");
    }

    SECTION("handles multiple dots: removes only last extension")
    {
        CHECK(withoutExtension("my.movie.mkv") == "my.movie");
    }

    SECTION("no extension: returns unchanged")
    {
        CHECK(withoutExtension("nodot") == "nodot");
    }
}

TEST_CASE("sortFilenameList", "[media]")
{
    SECTION("sorts alphabetically ignoring extension")
    {
        QStringList files = {"c.mkv", "a.avi", "b.mp4"};
        sortFilenameList(files);
        CHECK(files[0] == "a.avi");
        CHECK(files[1] == "b.mp4");
        CHECK(files[2] == "c.mkv");
    }
}
