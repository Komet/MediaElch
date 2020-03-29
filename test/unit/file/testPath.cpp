#include "test/test_helpers.h"

#include "file/Path.h"

using namespace mediaelch;

TEST_CASE("DirectoryPath", "[path]")
{
    SECTION("default constructed path is invalid")
    {
        CHECK_FALSE(DirectoryPath().isValid());
        CHECK_FALSE(DirectoryPath("").isValid());
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
