#include "test/test_helpers.h"

#include "media/Path.h"

using namespace mediaelch;

TEST_CASE("DirectoryPath", "[media]")
{
    SECTION("default constructed value is invalid")
    {
        CHECK_FALSE(DirectoryPath().isValid());
    }

    SECTION("empty string is invalid")
    {
        CHECK_FALSE(DirectoryPath("").isValid());
    }

    SECTION("non-empty path is valid")
    {
        CHECK(DirectoryPath("/tmp").isValid());
    }

    SECTION("equality")
    {
        CHECK(DirectoryPath("/tmp") == DirectoryPath("/tmp"));
        CHECK(DirectoryPath("/tmp") != DirectoryPath("/var"));
    }

    SECTION("isParentFolderOf")
    {
        CHECK(DirectoryPath("/movies").isParentFolderOf(DirectoryPath("/movies/Action")));
        CHECK(DirectoryPath("/movies").isParentFolderOf(DirectoryPath("/movies")));
        CHECK_FALSE(DirectoryPath("/movies").isParentFolderOf(DirectoryPath("/tv")));
    }
}

TEST_CASE("FilePath", "[media]")
{
    SECTION("default constructed value is invalid")
    {
        CHECK_FALSE(FilePath().isValid());
    }

    SECTION("empty string is invalid")
    {
        CHECK_FALSE(FilePath("").isValid());
    }

    SECTION("non-empty path is valid")
    {
        CHECK(FilePath("/tmp/movie.mkv").isValid());
    }

    SECTION("equality")
    {
        CHECK(FilePath("/tmp/movie.mkv") == FilePath("/tmp/movie.mkv"));
        CHECK(FilePath("/tmp/movie.mkv") != FilePath("/tmp/other.mkv"));
    }

    SECTION("fileName returns the last path component")
    {
        CHECK(FilePath("/tmp/movie.mkv").fileName() == "movie.mkv");
    }

    SECTION("fileSuffix returns the extension")
    {
        CHECK(FilePath("/tmp/movie.mkv").fileSuffix() == "mkv");
    }
}
