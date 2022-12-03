#include "test/test_helpers.h"

#include "file_search/TvShowFileSearcher.h"

static SeasonNumber getSeasonNumber(QString filename)
{
    CAPTURE(filename);
    return TvShowFileSearcher::getSeasonNumber({filename});
}

TEST_CASE("TvShowFileSearcher parses season data", "[show][utils]")
{
    SECTION("single episode file")
    {
        CHECK(getSeasonNumber("dir/4.12.mov") == SeasonNumber(4));

        CHECK(getSeasonNumber("dir/S4E01.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/S04E01.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/S004E001.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/S14E01.mov") == SeasonNumber(14));
        CHECK(getSeasonNumber("dir/S142E001.mov") == SeasonNumber(142));
        CHECK(getSeasonNumber("dir/S1425E1005.mov") == SeasonNumber(1425));

        CHECK(getSeasonNumber("dir/Season.11-Episode.1.mov") == SeasonNumber(11));
        CHECK(getSeasonNumber("dir/Season-11 Episode-1.mov") == SeasonNumber(11));
        CHECK(getSeasonNumber("dir/Season 11 Episode 1.mov") == SeasonNumber(11));
        CHECK(getSeasonNumber("dir/Season_11_Episode_01.mov") == SeasonNumber(11));
        CHECK(getSeasonNumber("dir/Season14Episode16.mov") == SeasonNumber(14));

        CHECK(getSeasonNumber("dir/Name_S04E14.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/Name with space S04E14.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/Name_before_S14E14.mov") == SeasonNumber(14));
        CHECK(getSeasonNumber("dir/Name_before_S142E2.mov") == SeasonNumber(142));
        CHECK(getSeasonNumber("dir/Name_before_S1425E425.mov") == SeasonNumber(1425));

        CHECK(getSeasonNumber("dir/S14E04_Name.mov") == SeasonNumber(14));
        CHECK(getSeasonNumber("dir/S14E04 Name with space.mov") == SeasonNumber(14));
        CHECK(getSeasonNumber("dir/S14E04_Name_before.mov") == SeasonNumber(14));
        CHECK(getSeasonNumber("dir/S142E42_Name_before.mov") == SeasonNumber(142));
        CHECK(getSeasonNumber("dir/S1425E25_Name_before.mov") == SeasonNumber(1425));

        CHECK(getSeasonNumber("Oz/Oz.S04E01.Emerald City (720p)") == SeasonNumber(4));
    }

    SECTION("multi-episode file")
    {
        CHECK(getSeasonNumber("dir/S4E4-S4E5.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/S04E4-S04E05.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/S14E4-S14E115.mov") == SeasonNumber(14));
        CHECK(getSeasonNumber("dir/S004E004-S004E005.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/S004E004-S004E005-S04E15.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/S04E02E03E04E06.mov") == SeasonNumber(4));

        // Note: No episode ranges.
        CHECK(getSeasonNumber("dir/Name_S04E4-S04E6.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/S4E4 Name with space S04E05 Second name.mov") == SeasonNumber(4));
        CHECK(getSeasonNumber("dir/S14E14-S14E115 - Some name.mov") == SeasonNumber(14));
        CHECK(getSeasonNumber("dir/S04E004.S04E005-Another-Title.mov") == SeasonNumber(4));

        CHECK(getSeasonNumber("Oz/Oz.S04E01E02.Emerald City (720p)") == SeasonNumber(4));
    }
}
