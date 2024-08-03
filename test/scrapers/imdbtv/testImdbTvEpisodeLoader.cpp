#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h"
#include "test/helpers/scraper_helpers.h"
#include "test/mocks/settings/SettingsMock.h"
#include "test/scrapers/imdbtv/testImdbTvHelper.h"

using namespace mediaelch;
using namespace mediaelch::scraper;


TEST_CASE("ImdbTv scrapes episode details for The Simpsons S12E19", "[episode][ImdbTv][load_data]")
{
    SeasonNumber season(12);
    EpisodeNumber episodeNumber(19);
    ImdbId showId("tt0096697");
    ImdbId episodeId("tt0701133");

    SECTION("Loads minimal details with episode ImdbTv ID")
    {
        EpisodeIdentifier id(episodeId);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.imdbId() == ImdbId("tt0701133"));
        test::scraper::compareAgainstReference(
            episode, "scrapers/imdbtv/The-Simpsons-S12E19-tt0701133-minimal-details");

        // These fields should not be set
        CHECK_FALSE(episode.actors().hasActors());
    }

    SECTION("Loads minimal details with season and episode number")
    {
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.imdbId() == ImdbId("tt0701133"));
        test::scraper::compareAgainstReference(episode, "scrapers/imdbtv/The-Simpsons-S12E19-minimal-details");

        // These fields should not be set
        CHECK_FALSE(episode.actors().hasActors());
    }

    SECTION("Loads all details for The Simpsons S12E19")
    {
        SettingsMock mockSettings;
        ImdbTvConfiguration scraperConfig(mockSettings);
        ImdbTv imdbTv(scraperConfig);

        EpisodeIdentifier id(episodeId);
        EpisodeScrapeJob::Config config{id, Locale::English, imdbTv.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.imdbId() == ImdbId("tt0701133"));
        test::scraper::compareAgainstReference(episode, "scrapers/imdbtv/The-Simpsons-S12E19-tt0701133-all-details");
    }
}

TEST_CASE("ImdbTv scrapes episode details for Buffy", "[buffy][episode][ImdbTv][load_data]")
{
    SeasonNumber season(1);
    ImdbId showId("tt0118276");

    SECTION("Loads minimal details for episode number 00")
    {
        EpisodeNumber episodeNumber(0);
        ImdbId episodeId("tt0533518");
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.imdbId() == ImdbId("tt0533518"));
        test::scraper::compareAgainstReference(episode, "scrapers/imdbtv/Buffy-S01E00-minimal-details");
    }

    SECTION("Loads minimal details for episode number 01")
    {
        EpisodeNumber episodeNumber(1);
        ImdbId episodeId("tt0452716");
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.imdbId() == ImdbId("tt0452716"));
        test::scraper::compareAgainstReference(episode, "scrapers/imdbtv/Buffy-S01E01-minimal-details");
    }
}

TEST_CASE("ImdbTv scrapes episode details for 'All in the Family' S01E01", "[episode][ImdbTv][load_data]")
{
    SeasonNumber season(1);
    EpisodeNumber episodeNumber(1);
    ImdbId showId("tt0066626");
    ImdbId episodeId("tt0509891");

    SECTION("Loads details of episode S01E01")
    {
        EpisodeIdentifier id(episodeId);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.imdbId() == ImdbId("tt0509891"));
        test::scraper::compareAgainstReference(
            episode, "scrapers/imdbtv/All-in-the-Family-S01E01-tt0509891-minimal-details");

        // These fields should not be set
        CHECK_FALSE(episode.actors().hasActors());
    }
}
