#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"
#include "test/scrapers/testScraperHelpers.h"
#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("TmdbTv scrapes show details", "[show][TmdbTv][load_data]")
{
    waitForTmdbTvInitialized();

    SECTION("Loads minimal details for The Simpsons")
    {
        QSet<ShowScraperInfo> details{ShowScraperInfo::Title};
        ShowScrapeJob::Config config{ShowIdentifier("456"), Locale("en-US"), details};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("456"));
        test::compareAgainstReference(show, "scrapers/tmdbtv/The-Simpsons-tmdb456-minimal-details");
    }

    SECTION("Loads minimal details for Scrubs in other language")
    {
        ShowScrapeJob::Config config{ShowIdentifier("4556"), Locale("de-DE"), {ShowScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("4556"));
        test::compareAgainstReference(show, "scrapers/tmdbtv/Scrubs-tmdb4556-minimal-details-DE");
    }

    SECTION("Loads all details for Scrubs")
    {
        TmdbTv tvdb;
        ShowScrapeJob::Config config{ShowIdentifier("4556"), Locale("en-US"), tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("4556"));
        test::compareAgainstReference(show, "scrapers/tmdbtv/Scrubs-tmdb4556-all-details");
    }

    SECTION("Loads all details for Scrubs in another Language")
    {
        TmdbTv tvdb;
        ShowScrapeJob::Config config{ShowIdentifier("4556"), Locale("de-DE"), tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("4556"));
        test::compareAgainstReference(show, "scrapers/tmdbtv/Scrubs-tmdb4556-all-details-DE");
    }

    SECTION("Loads the full cast for Stargate SG-1")
    {
        // Test everything, including cast.
        TmdbTv tvdb;
        ShowScrapeJob::Config config{ShowIdentifier("4629"), Locale("en-US"), tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("4629"));
        REQUIRE(show.imdbId() == ImdbId("tt0118480"));

        // We used TMDb's `credits` field in the past. This field only contains
        // the actors of the _last_ season.  Newer MediaElch versions use
        // `aggregate_credits`, which also includes e.g. "Richard Dean Anderson".
        // There are only 6 actors listed for the last season.
        const auto& actors = show.actors().actors();
        // Yes, there are really 680 actors.
        REQUIRE(actors.size() > 680);
        CHECK(actors[1]->name == "Richard Dean Anderson");
        CHECK(actors[1]->role == "Jack O'Neill");
        CHECK(actors[1]->id == "26085");
        CHECK(actors[1]->thumb == "https://image.tmdb.org/t/p/original/w9Wi0OUEFGy9vMUpiZjj9GLzpag.jpg");

        test::compareAgainstReference(show, "scrapers/tmdbtv/Stargate-SG-1-tmdb4629");
    }
}
