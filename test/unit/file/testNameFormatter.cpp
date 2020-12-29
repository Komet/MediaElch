#include "test/test_helpers.h"

#include "file/NameFormatter.h"


TEST_CASE("NameFormatter formats names", "[rename]")
{
    // Just some words to search for.
    QStringList defaultExcludeWords{
        "ac3", "dts", "divx5", "dsr", "dvd", "dvdrip", "fs", "hdtv", "480i", "720p", "1080p", "bluray", "h264", "mkv"};

    SECTION("excludeWords works as expected")
    {
        SECTION("Removes default excluded words from name")
        {
            NameFormatter nf(defaultExcludeWords);
            // end
            CHECK(nf.excludeWords("my-movie-480i") == "my-movie");
            CHECK(nf.excludeWords("my-movie.480i") == "my-movie");
            CHECK(nf.excludeWords("my-movie.ac3.dsr.480i") == "my-movie");
            // start
            CHECK(nf.excludeWords("480i-my-movie") == "my-movie");
            CHECK(nf.excludeWords("[480i]my-movie") == "my-movie");
            // middle
            CHECK(nf.excludeWords("my-480i-movie") == "my movie");
            CHECK(nf.excludeWords("my.dsr.divx5.480i.movie") == "my movie");
            CHECK(nf.excludeWords("my[480i]movie") == "my movie");
        }

        SECTION("Only match basic words and not regex special characters")
        {
            NameFormatter nf({".?", "(480i)", ".+"});
            CHECK(nf.excludeWords("my.movie.480i") == "my.movie.480i");
            CHECK(nf.excludeWords("my.movie.?.?.?480i") == "my.movie480i");
            CHECK(nf.excludeWords("my(480i)movie") == "mymovie");
            CHECK(nf.excludeWords("480i-..+my-movie") == "480i-.my-movie");
        }

        SECTION("Match braces")
        {
            NameFormatter nf({"(", ")", "<", ">", "."});
            CHECK(nf.excludeWords("my<movie>480i") == "mymovie480i");
            CHECK(nf.excludeWords("my(480i)movie") == "my480imovie");
            CHECK(nf.excludeWords("my.480i.movie") == "my480imovie");
            CHECK(nf.excludeWords("480i-.+my-movie") == "480i-+my-movie");
        }

        SECTION("Removes trailing _ -")
        {
            NameFormatter nf(defaultExcludeWords);
            CHECK(nf.excludeWords(" my-movie-480i_-_480i-") == "my-movie");
        }

        SECTION("Removes words with Unicode characters")
        {
            NameFormatter nf({"señor", "für", "✓"});
            CHECK(nf.excludeWords("my-señor-movie") == "my movie");
            CHECK(nf.excludeWords("my-señor-movie-für-mich.✓") == "my movie mich");
        }

        SECTION("Case-Insensitive")
        {
            NameFormatter nf({"ä", "HELLO", "HELLO.LONG", "dvd.HD"});
            // spaces after the replaced words because the "smart" regex is used
            CHECK(nf.excludeWords("my.Ä.movie.Ä") == "my movie");
            CHECK(nf.excludeWords("my.movie.hElLo.480i") == "my.movie 480i");
            // dot before 480i because the longer variant should simply be replaced
            CHECK(nf.excludeWords("my.movie.hElLo.loNg.480i") == "my.movie.480i");
            CHECK(nf.excludeWords("my.DVD.hd.movie") == "my.movie");
        }

        SECTION("Removes multiple dots and dashes")
        {
            NameFormatter nf({});
            CHECK(nf.excludeWords("my....movie..........in.480i") == "my.movie.in.480i");
            CHECK(nf.excludeWords("my--480i---------movie") == "my-480i-movie");
        }
    }

    SECTION("formatName works as expected")
    {
        NameFormatter nf(defaultExcludeWords);
        SECTION("replaces dots and underscore")
        {
            CHECK(nf.formatName("_movie_with_some_-_title.ac3", true, true) == "movie with some - title");
            CHECK(nf.formatName(".movie_with_some_-_title.ac3  - ", true, true) == "movie with some - title");
        }
        SECTION("replaces empty parentheses")
        {
            CHECK(nf.formatName("_movie_with_some_-_title( ac3 )  - ", true, true) == "movie with some - title");
            CHECK(nf.formatName("_movie_with_some_-_title( _ac3__ )  - ", true, true) == "movie with some - title");
        }
    }

    SECTION("removeParts works as expected")
    {
        NameFormatter nf({});
        SECTION("removes the last part")
        {
            CHECK(nf.removeParts("my-movie-part1") == "my-movie");
            CHECK(nf.removeParts("my-movie cd 1") == "my-movie");
            CHECK(nf.removeParts("my-movie cd_1") == "my-movie");
            CHECK(nf.removeParts("my-movie a.") == "my-movie");
            CHECK(nf.removeParts("my-movie_b") == "my-movie");
            CHECK(nf.removeParts("my-movie-_-f") == "my-movie");
            CHECK(nf.removeParts("my-movie  - part.45-") == "my-movie");
        }
        SECTION("does not remove middle part")
        {
            CHECK(nf.removeParts("my-part1-movie") == "my-part1-movie");
            CHECK(nf.removeParts("my-cd.1-movie") == "my-cd.1-movie");
        }
    }
}
