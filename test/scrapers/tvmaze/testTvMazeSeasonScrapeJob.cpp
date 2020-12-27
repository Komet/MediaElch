#include "test/test_helpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "scrapers/tv_show/tvmaze/TvMazeSeasonScrapeJob.h"
#include "test/scrapers/tvmaze/testTvMazeHelper.h"
#include "tv_shows/TvShowEpisode.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

static void scrapeSeasonSync(SeasonScrapeJob* scrapeJob)
{
    QEventLoop loop;
    QEventLoop::connect(scrapeJob, &SeasonScrapeJob::sigFinished, [&](SeasonScrapeJob* /*unused*/) {
        CAPTURE(scrapeJob->error().message);
        REQUIRE(!scrapeJob->hasError());
        loop.quit();
    });
    scrapeJob->execute();
    loop.exec();
}

TEST_CASE("TvMaze scrapes episode details for The Simpsons Season 12", "[season][TvMaze][load_data]")
{
    SeasonNumber season(12);
    TvMazeId showId("83");

    QString episodeTitle_s12e19 = "I'm Goin' to Praise Land";
    TvMazeId tvmazeId_s12e19("5264");
    TvDbId tvdbId_s12e19("55719");
    ImdbId imdbId_s12e19("tt0701133");

    const auto checkCommonFields = [&](const TvShowEpisode& episode) {
        // Title is requested, IDs are always set.
        CHECK(episode.tvmazeId() == tvmazeId_s12e19);
        CHECK(episode.firstAired() == QDate(2001, 05, 06));
    };

    SECTION("Loads minimal episode details for specific season")
    {
        SeasonScrapeJob::Config config{ShowIdentifier(showId),
            Locale::English,
            {SeasonNumber(12)},
            SeasonOrder::Aired,
            {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TvMazeSeasonScrapeJob>(getTvMazeApi(), config);
        scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        CHECK(episodes.size() == 21); // Season 12 is scraped and has all seasons

        const auto* episode = episodes[{SeasonNumber(12), EpisodeNumber(19)}];
        REQUIRE(episode != nullptr);
        checkCommonFields(*episode);
        CHECK(episode->title() == episodeTitle_s12e19);
        CHECK_THAT(episode->overview(), StartsWith("With Homer's help, Ned Flanders tries"));
    }

    SECTION("Loads minimal episode details for all seasons")
    {
        SeasonScrapeJob::Config config{
            ShowIdentifier(showId), Locale::English, {}, SeasonOrder::Aired, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TvMazeSeasonScrapeJob>(getTvMazeApi(), config);
        scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        CHECK(episodes.size() >= 690); // There are >30 seasons

        const auto* episode = episodes[{SeasonNumber(12), EpisodeNumber(19)}];
        REQUIRE(episode != nullptr);
        checkCommonFields(*episode);
        CHECK(episode->title() == episodeTitle_s12e19);
        CHECK_THAT(episode->overview(), StartsWith("With Homer's help, Ned Flanders tries"));
    }
}
