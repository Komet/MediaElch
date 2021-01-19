#include "test/test_helpers.h"

#include "file/FilenameUtils.h"

TEST_CASE("stackedBasedName", "[filename]")
{
    using namespace mediaelch::file;
    SECTION("Removes default excluded words from name")
    {
        // TODO: These tests currently fail
        // CHECK(stackedBaseName("/my/path/to/movie.mkv/captain.marvel.mk4") == "captain.marvel");
        // CHECK(stackedBaseName("/my/path/to/movie.mkv/captain.america.mk4") == "captain.america");
        CHECK(stackedBaseName("/my/path/to/movie.mkv/captain.america.part1.mk4") == "/my/path/to/movie.mkv/captain.america");
        CHECK(stackedBaseName("/my/path/to/movie.mkv/captain.america.pt1.mk4") == "/my/path/to/movie.mkv/captain.america");
        CHECK(stackedBaseName("/my/path/to/movie.mkv/captain.america.part-1.mk4") == "/my/path/to/movie.mkv/captain.america");
    }
}
