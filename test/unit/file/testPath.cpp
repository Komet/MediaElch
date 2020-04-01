#include "test/test_helpers.h"

#include "file/Path.h"

using namespace mediaelch;

TEST_CASE("DirectoryPath", "[path]")
{
    const QString absolute = "/tmp/example/path";
    const QString relative = "./tmp/example/path";

    SECTION("default constructed path is invalid")
    {
        CHECK_FALSE(DirectoryPath().isValid());
        CHECK_FALSE(DirectoryPath("").isValid());
    }

    SECTION("toString() returns a normalized absolute path") {
        CHECK(DirectoryPath(absolute).toString() == absolute);
        CHECK(DirectoryPath(relative).toString() != absolute);
        CHECK_THAT(DirectoryPath(relative).toString(), StartsWith("/"));
    }
}

TEST_CASE("FilePath", "[path]")
{
    SECTION("default constructed path is invalid")
    {
        CHECK_FALSE(FilePath().isValid());
        CHECK_FALSE(FilePath("").isValid());
    }
}
