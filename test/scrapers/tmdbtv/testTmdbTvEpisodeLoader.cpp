#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvConfiguration.h"
#include "scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.h"
#include "test/mocks/settings/SettingsMock.h"

#include "test/helpers/scraper_helpers.h"
#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("TmdbTv scrapes episode details for The Simpsons S12E19", "[episode][TmdbTv][load_data]")
{
    waitForTmdbTvInitialized();

    // Correct details for the episode
    SeasonNumber season(12);
    EpisodeNumber episodeNumber(19);
    TmdbId showId("456");
    TvDbId tvdbId("55719");
    TmdbId tmdbId("62494");
    ImdbId imdbId("tt0701133");

    SECTION("Loads minimal details with season and episode number")
    {
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("en-US"), {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvEpisodeScrapeJob>(getTmdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tmdbId() == tmdbId);
        REQUIRE(episode.imdbId() == ImdbId("tt0701133"));
        test::scraper::compareAgainstReference(episode, "scrapers/tmdbtv/The-Simpsons-tmdb456-S12E19-minimal-details");
    }

    SECTION("Loads minimal details for The Simpsons S12E19 in other language")
    {
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvEpisodeScrapeJob>(getTmdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tmdbId() == tmdbId);
        REQUIRE(episode.imdbId() == ImdbId("tt0701133"));
        test::scraper::compareAgainstReference(
            episode, "scrapers/tmdbtv/The-Simpsons-tmdb456-S12E19-minimal-details-DE");
    }

    SECTION("Loads all details for The Simpsons S12E19")
    {
        SettingsMock mockedSettings;
        TmdbTvConfiguration scraperConfig(mockedSettings);
        TmdbTv tmdb(scraperConfig);
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("en-US"), tmdb.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TmdbTvEpisodeScrapeJob>(getTmdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tmdbId() == tmdbId);
        REQUIRE(episode.imdbId() == ImdbId("tt0701133"));
        test::scraper::compareAgainstReference(episode, "scrapers/tmdbtv/The-Simpsons-tmdb456-S12E19-all-details");
    }

    SECTION("Loads all details for The Simpsons S12E19 in another Language")
    {
        SettingsMock mockedSettings;
        TmdbTvConfiguration scraperConfig(mockedSettings);
        TmdbTv tmdb(scraperConfig);
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), tmdb.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TmdbTvEpisodeScrapeJob>(getTmdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tmdbId() == tmdbId);
        REQUIRE(episode.imdbId() == ImdbId("tt0701133"));
        test::scraper::compareAgainstReference(episode, "scrapers/tmdbtv/The-Simpsons-tmdb456-S12E19-all-details-DE");
    }
}
