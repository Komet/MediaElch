#include "test/test_helpers.h"

#include "file_search/TvShowFileSearcher.h"

static EpisodeNumber getEpisodeNumber(QString filename)
{
    CAPTURE(filename);
    auto numbers = TvShowFileSearcher::getEpisodeNumbers({filename});
    REQUIRE(numbers.size() == 1);
    return numbers.at(0);
}

static QVector<EpisodeNumber> getEpisodeNumbers(QString filename)
{
    CAPTURE(filename);
    auto numbers = TvShowFileSearcher::getEpisodeNumbers({filename});
    REQUIRE(!numbers.isEmpty());
    return numbers;
}

static QVector<EpisodeNumber> episodeList(QVector<int> episodes)
{
    QVector<EpisodeNumber> list;
    for (const int episode : episodes) {
        list.push_back(EpisodeNumber(episode));
    }
    return list;
}

TEST_CASE("TvShowFileSearcher parses episode data", "[show][utils]")
{
    SECTION("single episode file")
    {
        CHECK(getEpisodeNumber("dir/S01E4.mov") == EpisodeNumber(4));
        CHECK(getEpisodeNumber("dir/S01E04.mov") == EpisodeNumber(4));
        CHECK(getEpisodeNumber("dir/S01E004.mov") == EpisodeNumber(4));
        CHECK(getEpisodeNumber("dir/S01E14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/S01E142.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/S01E1425.mov") == EpisodeNumber(1425));

        CHECK(getEpisodeNumber("dir/Season.01-Episode.142.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/Season.01 Episode.142.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/Season01Episode14.mov") == EpisodeNumber(14));

        CHECK(getEpisodeNumber("dir/Name_S01E14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/Name with space S01E14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/Name_before_S01E14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/Name_before_S01E142.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/Name_before_S01E1425.mov") == EpisodeNumber(1425));

        CHECK(getEpisodeNumber("dir/S01E14_Name.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/S01E14 Name with space.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/S01E14_Name_before.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/S01E142_Name_before.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/S01E1425_Name_before.mov") == EpisodeNumber(1425));

        // In v2.8.8 we did not correctly set the offset for the multi-part episode check
        // See https://github.com/Komet/MediaElch/pull/1302
        CHECK(getEpisodeNumber("Oz/Oz.S01E01.Emerald City (720p)") == EpisodeNumber(1));

		// Check for TV show without season number
        CHECK(getEpisodeNumber("dir/ep4.mov") == EpisodeNumber(4));
        CHECK(getEpisodeNumber("dir/ep04.mov") == EpisodeNumber(4));
        CHECK(getEpisodeNumber("dir/ep004.mov") == EpisodeNumber(4));
        CHECK(getEpisodeNumber("dir/ep14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/ep142.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/ep1425.mov") == EpisodeNumber(1425));

        CHECK(getEpisodeNumber("dir/ep_4.mov") == EpisodeNumber(4));
        CHECK(getEpisodeNumber("dir/ep_04.mov") == EpisodeNumber(4));
        CHECK(getEpisodeNumber("dir/ep_004.mov") == EpisodeNumber(4));
        CHECK(getEpisodeNumber("dir/ep_14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/ep_142.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/ep_1425.mov") == EpisodeNumber(1425));

        CHECK(getEpisodeNumber("dir/Name_ep14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/Name with space ep14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/Name_before_ep14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/Name_before_ep142.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/Name_before_ep1425.mov") == EpisodeNumber(1425));

        CHECK(getEpisodeNumber("dir/Name_ep_14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/Name with space ep_14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/Name_before_ep_14.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/Name_before_ep_142.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/Name_before_ep_1425.mov") == EpisodeNumber(1425));

        CHECK(getEpisodeNumber("dir/ep14_Name.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/ep14 Name with space.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/ep14_Name_before.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/ep142_Name_before.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/ep1425_Name_before.mov") == EpisodeNumber(1425));

        CHECK(getEpisodeNumber("dir/ep_14_Name.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/ep_14 Name with space.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/ep_14_Name_before.mov") == EpisodeNumber(14));
        CHECK(getEpisodeNumber("dir/ep_142_Name_before.mov") == EpisodeNumber(142));
        CHECK(getEpisodeNumber("dir/ep_1425_Name_before.mov") == EpisodeNumber(1425));
    }

    SECTION("multi-episode file")
    {
        CHECK(getEpisodeNumbers("dir/S01E4-S01E5.mov") == episodeList({4, 5}));
        CHECK(getEpisodeNumbers("dir/S01E04-S01E05.mov") == episodeList({4, 5}));
        CHECK(getEpisodeNumbers("dir/S01E14-S01E115.mov") == episodeList({14, 115}));
        CHECK(getEpisodeNumbers("dir/S01E004-S01E005.mov") == episodeList({4, 5}));
        CHECK(getEpisodeNumbers("dir/S01E004-S01E005-S01E15.mov") == episodeList({4, 5, 15}));
        CHECK(getEpisodeNumbers("dir/S01E02E03E04E06.mov") == episodeList({2, 3, 4, 6}));

        // Note: No episode ranges.
        CHECK(getEpisodeNumbers("dir/Name_S01E4-S01E6.mov") == episodeList({4, 6}));
        CHECK(getEpisodeNumbers("dir/S01E4 Name with space S01E05 Second name.mov") == episodeList({4, 5}));
        CHECK(getEpisodeNumbers("dir/S01E14-S01E115 - Some name.mov") == episodeList({14, 115}));
        CHECK(getEpisodeNumbers("dir/S01E004.S01E005-Another-Title.mov") == episodeList({4, 5}));

        CHECK(getEpisodeNumbers("Oz/Oz.S01E01E02.Emerald City (720p)") == episodeList({1, 2}));

		// Check for TV show without season number
        CHECK(getEpisodeNumbers("dir/ep4-ep5.mov") == episodeList({4, 5}));
        CHECK(getEpisodeNumbers("dir/ep04-ep05.mov") == episodeList({4, 5}));
        CHECK(getEpisodeNumbers("dir/ep14-ep115.mov") == episodeList({14, 115}));
        CHECK(getEpisodeNumbers("dir/ep004-ep005.mov") == episodeList({4, 5}));

        CHECK(getEpisodeNumbers("dir/Name_ep4-ep6.mov") == episodeList({4, 6}));
        CHECK(getEpisodeNumbers("dir/ep4 Name with space ep05 Second name.mov") == episodeList({4, 5}));
        CHECK(getEpisodeNumbers("dir/ep14-ep115 - Some name.mov") == episodeList({14, 115}));
	}
}
