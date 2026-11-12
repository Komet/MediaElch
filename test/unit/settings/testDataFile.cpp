#include "test/test_helpers.h"

#include "data/tv_show/SeasonNumber.h"
#include "globals/Globals.h"
#include "settings/DataFile.h"

TEST_CASE("DataFile::saveFileName", "[settings]")
{
    DataFile nfo(DataFileType::MovieNfo, "<baseFileName>.nfo", 0);
    DataFile poster(DataFileType::MoviePoster, "<baseFileName>-poster.jpg", 0);

    SECTION("single file: strips video extension")
    {
        CHECK(nfo.saveFileName("movie.mkv") == "movie.nfo");
        CHECK(poster.saveFileName("movie.mkv") == "movie-poster.jpg");
    }

    SECTION("stacked multi-edition: strips video extension (not movie.mkv.nfo)")
    {
        CHECK(nfo.saveFileName("movie.mkv", SeasonNumber::NoSeason, true) == "movie.nfo");
        CHECK(poster.saveFileName("movie.mkv", SeasonNumber::NoSeason, true) == "movie-poster.jpg");

        CHECK(nfo.saveFileName("The.Dark.Knight.mkv", SeasonNumber::NoSeason, true) == "The.Dark.Knight.nfo");
    }

    SECTION("stacked cd1/cd2: uses stacked basename without extension")
    {
        CHECK(nfo.saveFileName("movie-cd1.avi", SeasonNumber::NoSeason, true) == "movie.nfo");
        CHECK(nfo.saveFileName("movie-cd2.avi", SeasonNumber::NoSeason, true) == "movie.nfo");
        CHECK(poster.saveFileName("movie-part1.mkv", SeasonNumber::NoSeason, true) == "movie-poster.jpg");
    }

    SECTION("stacked edition suffix: strips only the video extension")
    {
        CHECK(nfo.saveFileName("Title - US.mp4", SeasonNumber::NoSeason, true) == "Title - US.nfo");
        CHECK(poster.saveFileName("Clerks (1994) [tmdbid-2292] - First Cut.mkv", SeasonNumber::NoSeason, true)
            == "Clerks (1994) [tmdbid-2292] - First Cut-poster.jpg");
    }

    SECTION("stacked Part I (roman): stacking strips part → folder-level nfo basename")
    {
        // stackedBaseName treats "Part I" as a disc marker (I in [0-9a-f]).
        const QString video =
            "Jay and Silent Bob Get Irish - The Swearing o' the Green! (2012) [tmdbid-177361] - Part I.mkv";
        const QString expected =
            "Jay and Silent Bob Get Irish - The Swearing o' the Green! (2012) [tmdbid-177361].nfo";
        CHECK(nfo.saveFileName(video, SeasonNumber::NoSeason, true) == expected);
        CHECK_FALSE(nfo.saveFileName(video, SeasonNumber::NoSeason, true).endsWith(".mkv.nfo"));
    }

    SECTION("nfoFilePath-style: multi-file movie finds Title.nfo from Title.mkv")
    {
        const QString candidate = nfo.saveFileName("Title.mkv", SeasonNumber::NoSeason, true);
        CHECK(candidate == "Title.nfo");
        CHECK(candidate != "Title.mkv.nfo");
    }
}

TEST_CASE("DataFile::sidecarFileNameCandidates", "[settings]")
{
    const QVector<DataFile> nfoPatterns{DataFile(DataFileType::MovieNfo, "<baseFileName>.nfo", 0)};
    const QVector<DataFile> posterPatterns{DataFile(DataFileType::MoviePoster, "<baseFileName>-poster.jpg", 0)};

    SECTION("strict: only primary stacked candidate")
    {
        const QString folder = "Faust (1926) [tmdbid-10728]";
        const QStringList c = DataFile::sidecarFileNameCandidates(
            nfoPatterns, folder + " - Sub.avi", true, folder, SidecarLookupOptions::strict());

        REQUIRE(c.size() == 1);
        CHECK(c.first() == folder + " - Sub.nfo");
        CHECK_FALSE(c.contains(folder + ".nfo"));
        CHECK_FALSE(c.contains("movie.nfo"));
    }

    SECTION("movie load: folder.nfo before movie.nfo (prefer TMM over stale movie.nfo)")
    {
        const QString folder = "Faust (1926) [tmdbid-10728]";
        const QStringList c = DataFile::sidecarFileNameCandidates(
            nfoPatterns, folder + " - Sub.avi", true, folder, SidecarLookupOptions::forMovieLoad());

        REQUIRE(c.contains(folder + " - Sub.nfo"));
        REQUIRE(c.contains(folder + ".nfo"));
        REQUIRE(c.contains("movie.nfo"));
        CHECK(c.indexOf(folder + " - Sub.nfo") < c.indexOf(folder + ".nfo"));
        CHECK(c.indexOf(folder + ".nfo") < c.indexOf("movie.nfo"));
    }

    SECTION("primary stacked candidate without video extension")
    {
        const QStringList c = DataFile::sidecarFileNameCandidates(
            nfoPatterns, "Title.mkv", true, "Title", SidecarLookupOptions::forMovieLoad());
        REQUIRE_FALSE(c.isEmpty());
        CHECK(c.first() == "Title.nfo");
        CHECK_FALSE(c.contains("Title.mkv.nfo"));
    }

    SECTION("cd1: non-stacked retry and folder basename")
    {
        const QStringList c = DataFile::sidecarFileNameCandidates(
            nfoPatterns, "movie-cd1.avi", true, "Movie Folder", SidecarLookupOptions::forConcertLoad());
        CHECK(c.contains("movie.nfo"));     // stacked base
        CHECK(c.contains("movie-cd1.nfo")); // non-stacked
        CHECK(c.contains("Movie Folder.nfo"));
        CHECK(c.count("movie.nfo") == 1);
    }

    SECTION("deduplicates identical candidates")
    {
        const QStringList c = DataFile::sidecarFileNameCandidates(
            nfoPatterns, "Title.mkv", true, "Title", SidecarLookupOptions::forMovieLoad());
        CHECK(c.count("Title.nfo") == 1);
    }

    SECTION("Part I video: primary already folder-level; folder fallback still listed once")
    {
        const QString folder = "Jay and Silent Bob Get Irish - The Swearing o' the Green! (2012) [tmdbid-177361]";
        const QString video = folder + " - Part I.mkv";
        const QStringList c = DataFile::sidecarFileNameCandidates(
            nfoPatterns, video, true, folder, SidecarLookupOptions::forMovieLoad());
        CHECK(c.first() == folder + ".nfo");
        CHECK(c.count(folder + ".nfo") == 1);
    }

    SECTION("image load: folder-level poster candidate for edition-first file")
    {
        const QString folder = "Faust (1926) [tmdbid-10728]";
        const QStringList c = DataFile::sidecarFileNameCandidates(
            posterPatterns, folder + " - Sub.avi", true, folder, SidecarLookupOptions::forImageLoad());

        CHECK(c.contains(folder + " - Sub-poster.jpg"));
        CHECK(c.contains(folder + "-poster.jpg"));
        CHECK_FALSE(c.contains("movie.nfo"));
        CHECK(c.indexOf(folder + " - Sub-poster.jpg") < c.indexOf(folder + "-poster.jpg"));
    }

    SECTION("episode load: non-stacked only, no folder/movie.nfo")
    {
        const QStringList c = DataFile::sidecarFileNameCandidates(
            nfoPatterns, "Show.S01E01-cd1.mkv", true, "Season 01", SidecarLookupOptions::forEpisodeLoad());
        CHECK(c.contains("Show.S01E01.nfo"));     // stacked
        CHECK(c.contains("Show.S01E01-cd1.nfo")); // non-stacked
        CHECK_FALSE(c.contains("Season 01.nfo"));
        CHECK_FALSE(c.contains("movie.nfo"));
    }
}
