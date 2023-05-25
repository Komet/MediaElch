#include "test/test_helpers.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h"
#include "test/helpers/scraper_helpers.h"
#include "test/scrapers/imdbtv/testImdbTvHelper.h"

using namespace mediaelch;
using namespace mediaelch::scraper;


TEST_CASE("ImdbTv scrapes episode details for The Simpsons S12E19", "[episode][ImdbTv][load_data]")
{
    SeasonNumber season(12);
    EpisodeNumber episodeNumber(19);
    ImdbId showId("tt0096697");
    ImdbId imdbId("tt0701133");

    SECTION("Loads minimal details with episode ImdbTv ID")
    {
        EpisodeIdentifier id(imdbId);
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
        ImdbTv imdbTv;
        EpisodeIdentifier id(imdbId);
        EpisodeScrapeJob::Config config{id, Locale::English, imdbTv.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.imdbId() == ImdbId("tt0701133"));
        test::scraper::compareAgainstReference(episode, "scrapers/imdbtv/The-Simpsons-S12E19-tt0701133-all-details");
    }
}
