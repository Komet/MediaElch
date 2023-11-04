#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"

using namespace mediaelch::scraper;

TEST_CASE("ImdbTvEpisodeParser extracts an episode id from season page", "[episode][ImdbTv][parse_data]")
{
    // Taken from https://www.imdb.com/title/tt0096697/episodes?season=4 on 2023-11-04
    QString episodeEntryHtml =
        R"(site=web"},{"id":"tt0773651","type":"tvEpisode","season":"4","episode":"18","titleText":"So It's Come to )"
        R"(This: A Simpsons Clip Show","releaseDate":{"month":4,"day":1,"year":1993,"__typename":)"
        R"(ReleaseDate"},"releaseYear":1993,"image")";

    ImdbId expectedEpisodeId("tt0773651");
    TvShowEpisode episode;
    episode.setSeason(SeasonNumber(4));
    episode.setEpisode(EpisodeNumber(18));

    ImdbTvEpisodeParser::parseIdFromSeason(episode, episodeEntryHtml);
    CHECK(episode.imdbId() == expectedEpisodeId);
}
