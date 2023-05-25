#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/tv_show/fernsehserien_de/FernsehserienDe.h"

#include "test/helpers/scraper_helpers.h"

#include <chrono>

using namespace mediaelch;
using namespace mediaelch::scraper;


TEST_CASE("FernsehserienDe scrapes seasons", "[season][FernsehserienDe][load_data]")
{
    auto api = std::make_unique<FernsehserienDeApi>();

    SECTION("Loads details for episodes of S5 of Black Mirror")
    {
        // I chose Black Mirror because they have only few episodes per season => faster load times

        // Correct details for the season
        const SeasonNumber season5(5);
        const int numberOfEpisodesInSeason5 = 3;

        SeasonScrapeJob::Config config{
            ShowIdentifier("black-mirror"), Locale("de-DE"), {season5}, SeasonOrder::Aired, allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<FernsehserienDeSeasonScrapeJob>(*api, config);
        test::scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        REQUIRE(episodes.size() == numberOfEpisodesInSeason5);
        test::scraper::compareAgainstReference(episodes, "scrapers/fernsehserien_de/Black-Mirror-S05");
    }

    SECTION("Loads details for all seasons of Black Mirror")
    {
        const int allEpisodesCount = 28; // as of 2023-06-03

        SeasonScrapeJob::Config config{
            ShowIdentifier("black-mirror"), Locale("de-DE"), {}, SeasonOrder::Aired, allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<FernsehserienDeSeasonScrapeJob>(*api, config);
        test::scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        REQUIRE(episodes.size() >= allEpisodesCount);
        test::scraper::compareAgainstReference(episodes, "scrapers/fernsehserien_de/Black-Mirror-all-seasons");
    }

    SECTION("Simply returns no result for TV show without episodes if _all_ episodes are loaded")
    {
        // Some french TV show; I've never heard of it, but it has a few non-ASCII characters.
        // Also, this show also has no episodes on fernsehserien.de!
        // https://www.fernsehserien.de/ya-pas-dage
        SeasonScrapeJob::Config config{
            ShowIdentifier("ya-pas-dage"), Locale("de-DE"), {}, SeasonOrder::Aired, allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<FernsehserienDeSeasonScrapeJob>(*api, config);
        test::scrapeSeasonSync(scrapeJob.get());

        CHECK(scrapeJob->episodes().isEmpty());
        CHECK_FALSE(scrapeJob->hasError());
    }

    SECTION("Simply returns no result for non-existent season")
    {
        // This show has no episodes on fernsehserien.de!
        SeasonScrapeJob::Config config{ShowIdentifier("ya-pas-dage"),
            Locale("de-DE"),
            {SeasonNumber(4)},
            SeasonOrder::Aired,
            allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<FernsehserienDeSeasonScrapeJob>(*api, config);
        test::scrapeSeasonSync(scrapeJob.get());

        CHECK(scrapeJob->episodes().isEmpty());
        CHECK_FALSE(scrapeJob->hasError());
    }
}
