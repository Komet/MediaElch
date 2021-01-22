#include "test/test_helpers.h"

#include "file/FilenameUtils.h"

TEST_CASE("stackedBasedName", "[filename]")
{
    using namespace mediaelch::file;

    // This test case currently only covers the bug reported in
    // https://github.com/Komet/MediaElch/issues/1194

    SECTION("Removes 'partN' from the filename with *nix filenames")
    {
        // Do not remove the filename if it's not split up
        CHECK(stackedBaseName("/path/to/movie.mkv/captain.marvel.mk4") == "/path/to/movie.mkv/captain.marvel.mk4");
        CHECK(stackedBaseName("/path/to/movie.mkv/captain.america.mk4") == "/path/to/movie.mkv/captain.america.mk4");

        CHECK(stackedBaseName("/path/to/movie.mkv/captain.america.part1.mk4") == "/path/to/movie.mkv/captain.america");
        CHECK(stackedBaseName("/path/to/movie.mkv/captain.marvel.pt1.ignored.mk4")
              == "/path/to/movie.mkv/captain.marvel");
        CHECK(stackedBaseName("/path/to/movie.mkv/captain.america.part-1.mk4") == "/path/to/movie.mkv/captain.america");
    }

    SECTION("Removes 'partN' from the filename with Windows filenames")
    {
        // Do not remove the filename if it's not split up
        CHECK(stackedBaseName("C:\\path\\to\\movie.mkv\\captain.marvel.mk4")
              == "C:\\path\\to\\movie.mkv\\captain.marvel.mk4");
        CHECK(stackedBaseName("C:\\path\\to\\movie.mkv\\captain.america.mk4")
              == "C:\\path\\to\\movie.mkv\\captain.america.mk4");

        CHECK(stackedBaseName("C:\\path\\to\\movie.mkv\\captain.america.part1.mk4")
              == "C:\\path\\to\\movie.mkv\\captain.america");
        CHECK(stackedBaseName("C:\\path\\to\\movie.mkv\\captain.marvel.pt1.ignored.mk4")
              == "C:\\path\\to\\movie.mkv\\captain.marvel");
        CHECK(stackedBaseName("C:\\path\\to\\movie.mkv\\captain.america.part-1.mk4")
              == "C:\\path\\to\\movie.mkv\\captain.america");
    }
}
