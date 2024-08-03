#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "scrapers/tv_show/tvmaze/TvMazeEpisodeScrapeJob.h"

#include "test/helpers/scraper_helpers.h"
#include "test/mocks/settings/SettingsMock.h"
#include "test/scrapers/tvmaze/testTvMazeHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

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
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tvmazeId() == episodeId);
        test::scraper::compareAgainstReference(episode, "scrapers/tvmaze/The-Simpsons-S12E19-minimal-details");
    }

    SECTION("Loads all details for The Simpsons S12E19 with season and episode number")
    {
        SettingsMock mockSettings;
        TvMazeConfiguration scraperConfig(mockSettings);
        TvMaze tvmaze(scraperConfig);
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, tvmaze.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TvMazeEpisodeScrapeJob>(getTvMazeApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tvmazeId() == episodeId);
        test::scraper::compareAgainstReference(episode, "scrapers/tvmaze/The-Simpsons-S12E19-all-details");
    }

    SECTION("Loads all details for The Simpsons S12E19 with its ID")
    {
        SettingsMock mockSettings;
        TvMazeConfiguration scraperConfig(mockSettings);
        TvMaze tvmaze(scraperConfig);
        EpisodeIdentifier id(episodeId.toString());
        EpisodeScrapeJob::Config config{id, Locale::English, tvmaze.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TvMazeEpisodeScrapeJob>(getTvMazeApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tvmazeId() == episodeId);
        test::scraper::compareAgainstReference(episode, "scrapers/tvmaze/The-Simpsons-tvmaze5264-all-details");
    }
}
