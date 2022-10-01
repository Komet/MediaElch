#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "scrapers/tv_show/tvmaze/TvMazeEpisodeScrapeJob.h"
#include "test/scrapers/tvmaze/testTvMazeHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

static void scrapeEpisodeSync(EpisodeScrapeJob* scrapeJob)
{
    QEventLoop loop;
    QEventLoop::connect(scrapeJob, &EpisodeScrapeJob::loadFinished, scrapeJob, [&](EpisodeScrapeJob* /*unused*/) {
        CAPTURE(scrapeJob->errorCode());
        CAPTURE(scrapeJob->errorString());
        CAPTURE(scrapeJob->errorText());
        REQUIRE(!scrapeJob->hasError());
        loop.quit();
    });
    scrapeJob->start();
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

    SECTION("Loads minimal details with season and episode number")
    {
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TvMazeEpisodeScrapeJob>(getTvMazeApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tvmazeId() == episodeId);
        test::compareAgainstReference(episode, "scrapers/tvmaze/The-Simpsons-S12E19-minimal-details");
    }

    SECTION("Loads all details for The Simpsons S12E19 with season and episode number")
    {
        TvMaze tvmaze;
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, tvmaze.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TvMazeEpisodeScrapeJob>(getTvMazeApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tvmazeId() == episodeId);
        test::compareAgainstReference(episode, "scrapers/tvmaze/The-Simpsons-S12E19-all-details");
    }

    SECTION("Loads all details for The Simpsons S12E19 with its ID")
    {
        TvMaze tvmaze;
        EpisodeIdentifier id(episodeId.toString());
        EpisodeScrapeJob::Config config{id, Locale::English, tvmaze.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TvMazeEpisodeScrapeJob>(getTvMazeApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tvmazeId() == episodeId);
        test::compareAgainstReference(episode, "scrapers/tvmaze/The-Simpsons-tvmaze5264-all-details");
    }
}
