#include "test/test_helpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "scrapers/tv_show/tvmaze/TvMazeEpisodeScrapeJob.h"
#include "test/scrapers/tvmaze/testTvMazeHelper.h"
#include "tv_shows/TvShowEpisode.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

static void scrapeEpisodeSync(EpisodeScrapeJob* scrapeJob)
{
    QEventLoop loop;
    QEventLoop::connect(scrapeJob, &EpisodeScrapeJob::sigFinished, [&](EpisodeScrapeJob* /*unused*/) {
        CAPTURE(scrapeJob->error().message);
        REQUIRE(!scrapeJob->hasError());
        loop.quit();
    });
    scrapeJob->execute();
    loop.exec();
}

TEST_CASE("TvMaze scrapes episode details for The Simpsons S12E19", "[episode][TvMaze][load_data]")
{
    // Correct details for the episode
    QString episodeTitle = "I'm Goin' to Praise Land";
    SeasonNumber season(12);
    EpisodeNumber episodeNumber(19);
    TvMazeId showId("83");
    TvMazeId episodeId("5264");

    const auto checkCommonFields = [&](TvShowEpisode& episode) {
        // Title is requested, ID is always set.
        CHECK(episode.tvmazeId() == episodeId);
        CHECK(episode.title() == "I'm Goin' to Praise Land");
        CHECK(episode.firstAired() == QDate(2001, 05, 06));
        CHECK(episode.episodeNumber() == episodeNumber);
        CHECK(episode.seasonNumber() == season);
    };

    SECTION("Loads minimal details with season and episode number")
    {
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TvMazeEpisodeScrapeJob>(getTvMazeApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        checkCommonFields(episode);
        CHECK(episode.title() == episodeTitle);
    }

    SECTION("Loads all details for The Simpsons S12E19 with season and episode number")
    {
        TvMaze tvmaze;
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, tvmaze.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TvMazeEpisodeScrapeJob>(getTvMazeApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        checkCommonFields(episode);
        CHECK_THAT(episode.overview(), StartsWith("With Homer's help, Ned Flanders"));
        CHECK(episode.thumbnail() == QUrl("http://static.tvmaze.com/uploads/images/original_untouched/69/173070.jpg"));
    }

    SECTION("Loads all details for The Simpsons S12E19 with its ID")
    {
        TvMaze tvmaze;
        EpisodeIdentifier id(episodeId.toString());
        EpisodeScrapeJob::Config config{id, Locale::English, tvmaze.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TvMazeEpisodeScrapeJob>(getTvMazeApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        checkCommonFields(episode);
        CHECK_THAT(episode.overview(), StartsWith("With Homer's help, Ned Flanders"));
        CHECK(episode.thumbnail() == QUrl("http://static.tvmaze.com/uploads/images/original_untouched/69/173070.jpg"));
    }
}
