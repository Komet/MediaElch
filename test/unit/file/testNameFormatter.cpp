#include "test/test_helpers.h"

#include "media/NameFormatter.h"


TEST_CASE("NameFormatter formats names", "[rename]")
{
    // Just some words to search for.
    QStringList defaultExcludeWords{
        "ac3",
        "dts",
        "divx5",
        "dsr",
        "dvd",
        "dvdrip",
        "fs",
        "hdtv",
        "480i",
        "720p",
        "1080p",
        "bluray",
        "h264",
        "mkv",
        "uhd", //
    };

    SECTION("excludeWords works as expected")
    {
        SECTION("Removes default excluded words from name")
        {
            NameFormatter::setExcludeWords(defaultExcludeWords);
            // end
            CHECK(NameFormatter::excludeWords("my-movie-480i") == "my-movie");
            CHECK(NameFormatter::excludeWords("my-movie.480i") == "my-movie");
            CHECK(NameFormatter::excludeWords("my-movie.ac3.dsr.480i") == "my-movie");
            // start
            CHECK(NameFormatter::excludeWords("480i-my-movie") == "my-movie");
            CHECK(NameFormatter::excludeWords("[480i]my-movie") == "my-movie");
            // middle
            CHECK(NameFormatter::excludeWords("my-480i-movie") == "my movie");
            CHECK(NameFormatter::excludeWords("my.dsr.divx5.480i.movie") == "my movie");
            CHECK(NameFormatter::excludeWords("my[480i]movie") == "my movie");

            // Issue #1773
            CHECK(NameFormatter::excludeWords("THE_MATRIX_(1999)_(BLURAY)") == "THE_MATRIX_(1999)");
            CHECK(NameFormatter::excludeWords("THE_MATRIX_(1999)_(UHD)") == "THE_MATRIX_(1999)");
        }

        SECTION("Only match basic words and not regex special characters")
        {
            NameFormatter::setExcludeWords({".?", "(480i)", ".+"});
            CHECK(NameFormatter::excludeWords("my.movie.480i") == "my.movie.480i");
            CHECK(NameFormatter::excludeWords("my.movie.?.?.?480i") == "my.movie480i");
            CHECK(NameFormatter::excludeWords("my(480i)movie") == "mymovie");
            CHECK(NameFormatter::excludeWords("480i-..+my-movie") == "480i-.my-movie");
        }

        SECTION("Match braces")
        {
            NameFormatter::setExcludeWords({"(", ")", "<", ">"});
            CHECK(NameFormatter::excludeWords("my<movie>480i") == "mymovie480i");
            CHECK(NameFormatter::excludeWords("my(480i)movie") == "my480imovie");
            CHECK(NameFormatter::excludeWords("my.480i.movie") == "my.480i.movie");
            CHECK(NameFormatter::excludeWords("480i-.+my-movie") == "480i-.+my-movie");
        }

        SECTION("Removes trailing _ -")
        {
            NameFormatter::setExcludeWords(defaultExcludeWords);
            CHECK(NameFormatter::excludeWords(" my-movie-480i_-_480i-") == "my-movie");
        }

        SECTION("Removes words with Unicode characters")
        {
            NameFormatter::setExcludeWords({"señor", "für", "✓"});
            CHECK(NameFormatter::excludeWords("my-señor-movie") == "my movie");
            CHECK(NameFormatter::excludeWords("my-señor-movie-für-mich.✓") == "my movie mich");
        }

        SECTION("Case-Insensitive")
        {
            NameFormatter::setExcludeWords({"ä", "HELLO", "HELLO.LONG", "dvd.HD"});
            // spaces after the replaced words because the "smart" regex is used
            CHECK(NameFormatter::excludeWords("my.Ä.movie.Ä") == "my movie");
            CHECK(NameFormatter::excludeWords("my.movie.hElLo.480i") == "my.movie 480i");
            // dot before 480i because the longer variant should simply be replaced
            CHECK(NameFormatter::excludeWords("my.movie.hElLo.loNg.480i") == "my.movie 480i");
            CHECK(NameFormatter::excludeWords("my.DVD.hd.movie") == "my movie");
        }

        SECTION("Removes multiple dots and dashes")
        {
            NameFormatter::setExcludeWords({});
            CHECK(NameFormatter::excludeWords("my....movie..........in.480i") == "my.movie.in.480i");
            CHECK(NameFormatter::excludeWords("my--480i---------movie") == "my-480i-movie");
        }
    }

    SECTION("formatName works as expected")
    {
        NameFormatter::setExcludeWords(defaultExcludeWords);
        SECTION("replaces dots and underscore")
        {
            CHECK(NameFormatter::formatName("_movie_with_some_-_title.ac3", true, true) == "movie with some - title");
            CHECK(
                NameFormatter::formatName(".movie_with_some_-_title.ac3  - ", true, true) == "movie with some - title");
        }
        SECTION("replaces empty parentheses")
        {
            CHECK(NameFormatter::formatName("_movie_with_some_-_title( ac3 )  - ", true, true)
                  == "movie with some - title");
            CHECK(NameFormatter::formatName("_movie_with_some_-_title( _ac3__ )  - ", true, true)
                  == "movie with some - title");
        }
    }

    SECTION("removeParts works as expected")
    {
        NameFormatter::setExcludeWords({});
        SECTION("removes the last part")
        {
            CHECK(NameFormatter::removeParts("my-movie-part1") == "my-movie");
            CHECK(NameFormatter::removeParts("my-movie cd 1") == "my-movie");
            CHECK(NameFormatter::removeParts("my-movie cd_1") == "my-movie");
            CHECK(NameFormatter::removeParts("my-movie a.") == "my-movie");
            CHECK(NameFormatter::removeParts("my-movie_b") == "my-movie");
            CHECK(NameFormatter::removeParts("my-movie-_-f") == "my-movie");
            CHECK(NameFormatter::removeParts("my-movie  - part.45-") == "my-movie");
        }
        SECTION("does not remove middle part")
        {
            CHECK(NameFormatter::removeParts("my-part1-movie") == "my-part1-movie");
            CHECK(NameFormatter::removeParts("my-cd.1-movie") == "my-cd.1-movie");
        }
    }
}
