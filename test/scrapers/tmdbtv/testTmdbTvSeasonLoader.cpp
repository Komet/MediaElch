#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.h"
#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

static void scrapeSeasonSync(SeasonScrapeJob* scrapeJob)
{
    QEventLoop loop;
    QEventLoop::connect(scrapeJob, &SeasonScrapeJob::loadFinished, scrapeJob, [&](SeasonScrapeJob* /*unused*/) {
        CAPTURE(scrapeJob->errorCode());
        CAPTURE(scrapeJob->errorString());
        CAPTURE(scrapeJob->errorText());
        REQUIRE(!scrapeJob->hasError());
        loop.quit();
    });
    scrapeJob->start();
    loop.exec();
}

TEST_CASE("TmdbTv scrapes episode details for The Simpsons Season 12", "[season][TmdbTv][load_data]")
{
    waitForTmdbTvInitialized();

    // Correct details for the season
    SeasonNumber season(12);
    TmdbId showId("456");

    SECTION("Loads minimal episode details for specific season")
    {
        SeasonScrapeJob::Config config{ShowIdentifier(showId),
            Locale::English,
            {SeasonNumber(12)},
            SeasonOrder::Aired,
            {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvSeasonScrapeJob>(getTmdbApi(), config);
        scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        CHECK(episodes.size() == 21); // Season 12 is scraped and has all seasons

        TvShowEpisode* episode = episodes[{SeasonNumber(12), EpisodeNumber(19)}];
        REQUIRE(episode != nullptr);
        REQUIRE(episode->tmdbId() == TmdbId("62494"));
        test::scraper::compareAgainstReference(*episode, "scrapers/tmdbtv/The-Simpsons-single-season-S12-E19");
    }

    SECTION("Loads minimal episode details for all seasons")
    {
        SeasonScrapeJob::Config config{
            ShowIdentifier(showId), Locale::English, {}, SeasonOrder::Aired, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvSeasonScrapeJob>(getTmdbApi(), config);
        scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        CHECK(episodes.size() >= 750); // There are >30 seasons

        TvShowEpisode* episode = episodes[{SeasonNumber(12), EpisodeNumber(19)}];
        REQUIRE(episode != nullptr);
        REQUIRE(episode->tmdbId() == TmdbId("62494"));
        test::scraper::compareAgainstReference(*episode, "scrapers/tmdbtv/The-Simpsons-all-seasons-S12-E19");
    }
}
