#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeParser.h"

using namespace mediaelch::scraper;

TEST_CASE("ImdbTvEpisodeParser extracts an episode id from season page", "[episode][ImdbTv][parse_data]")
{
    // Taken from https://www.imdb.com/title/tt0096697/episodes?season=4 on 2023-11-04
    QString episodeEntryHtml =
        R"raw("value":"Unknown"}],"episodes":{"items":[{"id":"tt0701142","type":"tvEpisode","season":"4","episode":"1","titleText":"Kamp Krusty",)raw"
        R"raw("releaseDate":{"month":4,"day":14,"year":1994,"__typename":"ReleaseDate"},"releaseYear":1994,"image":{"url":"https://m.media-amazon.com/)raw"
        R"raw("images/M/MV5BNmYwNGU0MjctYzYzYS00NGE2LWIzMGEtMWVhYjgyMmU2YmE1XkEyXkFqcGc@._V1_.jpg","maxHeight":1280,"maxWidth":853,"caption":"Nancy )raw"
        R"raw("Cartwright and Dan Castellaneta in Die Simpsons (1989)"},"plot":"Bart and Lisa attend \u0026quot;Kamp Krusty\u0026quot; but it is nothing like )raw"
        R"raw("they thought it would be; Homer\u0026#39;s hair grows back and he loses weight while the kids are away.","aggregateRating":8.5,"voteCount":4745,)raw"
        R"raw("canRate":true,"contributionUrl":"https://contribute.imdb.com/image/tt0701142/add?bus=imdb\u0026return_url=https%3A%2F%2Fwww.imdb.com%2Fclose_me\u0026site=web"},)raw"
        R"raw("{"id":"tt0701048","type":"tvEpisode","season":"4","episode":"2","titleText":"A Streetcar Named Marge","releaseDate":{"month":2,"day":16,"year":1993,)raw"
        R"raw(""__typename":"ReleaseDate"},"releaseYear":1993,"image":{"url":"https://m.media-amazon.com/images/M/MV5BODgyNjgyYmEtNmE2Zi00ZGFkLWE3MWMtNTE4NjhhMzMwZjA1)raw"
        R"raw("XkEyXkFqcGc@._V1_.jpg","maxHeight":576,"maxWidth":768,"caption":"Julie Kavner in Die)raw";

    ImdbId expectedEpisodeId("tt0701048");
    TvShowEpisode episode;
    episode.setSeason(SeasonNumber(4));
    episode.setEpisode(EpisodeNumber(2));

    ImdbTvEpisodeParser::parseIdFromSeason(episode, episodeEntryHtml);
    CHECK(episode.imdbId() == expectedEpisodeId);
}
