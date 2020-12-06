#include "test/test_helpers.h"

#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"
#include "tv_shows/TvShowEpisode.h"

using namespace mediaelch::scraper;

TEST_CASE("ImdbTvEpisodeParser extracts an episode id from season page", "[episode][ImdbTv][parse_data]")
{
    // Taken from https://www.imdb.com/title/tt0096697/episodes?season=4 on 2020-11-19
    QString episodeEntryHtml = R"(
<a href="/title/tt0773651/?ref_=ttep_ep18"
title="So It's Come to This: A Simpsons Clip Show" itemprop="url"> <div data-const="tt0773651" class="hover-over-image zero-z-index ">
<img width="224" height="126" class="zero-z-index" alt="So It's Come to This: A Simpsons Clip Show" src="https://m.media-amazon.com/images/M/MV5BMmVlZjM1N2QtNjZhZC00OTcwLWFiOWMtOTdhZTg0Njk5YmZmXkEyXkFqcGdeQXVyNjcwMzEzMTU@._V1_UX224_CR0,0,224,126_AL_.jpg">
<div>S4, Ep18</div>
</div>
</a>
    )";

    ImdbId expectedEpisodeId("tt0773651");
    TvShowEpisode episode;
    episode.setSeason(SeasonNumber(4));
    episode.setEpisode(EpisodeNumber(18));

    ImdbTvEpisodeParser::parseIdFromSeason(episode, episodeEntryHtml);
    CHECK(episode.imdbId() == expectedEpisodeId);
}
