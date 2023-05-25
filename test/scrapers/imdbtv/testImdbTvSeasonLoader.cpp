#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvSeasonScrapeJob.h"

#include "test/helpers/scraper_helpers.h"
#include "test/scrapers/imdbtv/testImdbTvHelper.h"

#include <chrono>

using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("ImdbTv scrapes seasons for Black Mirror", "[season][ImdbTv][load_data]")
{
    // I chose Black Mirror because they have only few episodes per season
    // => faster load times
    ImdbId showId("tt2085059");

    SECTION("Loads minimal details for all episodes of S5")
    {
        // Correct details for the season
        const SeasonNumber season5(5);
        const int numberOfEpisodesInSeason5 = 3;

        SeasonScrapeJob::Config config{
            ShowIdentifier(showId), Locale::English, {season5}, SeasonOrder::Aired, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<ImdbTvSeasonScrapeJob>(getImdbApi(), config);
        test::scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        REQUIRE(episodes.size() == numberOfEpisodesInSeason5);
        test::scraper::compareAgainstReference(episodes, "scrapers/imdbtv/Black-Mirror-S05");
    }

    // TODO: Test loading of all seasons
}
