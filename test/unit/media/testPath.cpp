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
        SECTION("the same folder should return true")
        {
            const QString parent = GENERATE(QString("/movies"), QString("/movies/"), QString("/movies/../movies"));
            const QString child = GENERATE(QString("/movies"), QString("/movies/"), QString("/movies/../movies"));

            CAPTURE(parent, child);
            CHECK(DirectoryPath(parent).isParentFolderOf(DirectoryPath(child)));
        }

        SECTION("a parent folder should return true for a child folder")
        {
            const QString parent = GENERATE(QString("/movies"), QString("/movies/"), QString("/movies/../movies"));
            const QString child = GENERATE(QString("/movies/Action"), QString("/movies/Action/Thriller"));

            CAPTURE(parent, child);
            CHECK(DirectoryPath(parent).isParentFolderOf(DirectoryPath(child)));
        }

        SECTION("the root folder should return true for itself and its children")
        {
            const QString child = GENERATE(QDir::rootPath(), QDir::root().filePath("movies"));

            CAPTURE(child);
            CHECK(DirectoryPath(QDir::rootPath()).isParentFolderOf(DirectoryPath(child)));
        }

        SECTION("an unrelated folder should return false")
        {
            const QString child = GENERATE(QString("/tv"), QString("/music/Action"));

            CAPTURE(child);
            CHECK_FALSE(DirectoryPath("/movies").isParentFolderOf(DirectoryPath(child)));
        }

        SECTION("a shared name prefix (excluding trailing slash) should return false")
        {
            const QString child = GENERATE(QString("/movies-extra"), QString("/movies-extra/Action"));

            CAPTURE(child);
            CHECK_FALSE(DirectoryPath("/movies").isParentFolderOf(DirectoryPath(child)));
        }

        SECTION("a child folder should return false for its parent folder")
        {
            const QString parent = GENERATE(QString("/movies/Action"), QString("/movies/Action/Thriller"));

            CAPTURE(parent);
            CHECK_FALSE(DirectoryPath(parent).isParentFolderOf(DirectoryPath("/movies")));
        }
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
