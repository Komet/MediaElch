#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvConfiguration.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"
#include "test/helpers/scraper_helpers.h"
#include "test/mocks/settings/SettingsMock.h"
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
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("456"));
        // Scrape locale is English → title is the English title.
        CHECK(show.englishTitle() == "The Simpsons");
        test::scraper::compareAgainstReference(show, "scrapers/tmdbtv/The-Simpsons-tmdb456-minimal-details");
    }

    SECTION("Loads minimal details for Scrubs in other language")
    {
        ShowScrapeJob::Config config{ShowIdentifier("4556"), Locale("de-DE"), {ShowScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("4556"));
        // original_language=en → english title from original_name (no extra EN request).
        CHECK(show.englishTitle() == "Scrubs");
        test::scraper::compareAgainstReference(show, "scrapers/tmdbtv/Scrubs-tmdb4556-minimal-details-DE");
    }

    SECTION("Loads all details for Scrubs")
    {
        SettingsMock mockedSettings;
        TmdbTvConfiguration scraperConfig(mockedSettings);
        TmdbTv tmdb(scraperConfig);
        ShowScrapeJob::Config config{ShowIdentifier("4556"), Locale("en-US"), tmdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("4556"));
        CHECK(show.englishTitle() == "Scrubs");
        test::scraper::compareAgainstReference(show, "scrapers/tmdbtv/Scrubs-tmdb4556-all-details");
    }

    SECTION("Loads all details for Scrubs in another Language")
    {
        SettingsMock mockedSettings;
        TmdbTvConfiguration scraperConfig(mockedSettings);
        TmdbTv tmdb(scraperConfig);
        ShowScrapeJob::Config config{ShowIdentifier("4556"), Locale("de-DE"), tmdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("4556"));
        CHECK(show.englishTitle() == "Scrubs");
        test::scraper::compareAgainstReference(show, "scrapers/tmdbtv/Scrubs-tmdb4556-all-details-DE");
    }

    SECTION("English title via extra EN request when original language is not en")
    {
        // Money Heist / La casa de papel (original_language=es) — requires loadEnglishTitle()
        ShowScrapeJob::Config config{ShowIdentifier("71446"), Locale("de-DE"), {ShowScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("71446"));
        // TMDb strings can drift; assert the EN-request path filled a distinct english title.
        REQUIRE_FALSE(show.englishTitle().isEmpty());
        CHECK(show.englishTitle() != show.originalTitle());
        CHECK(show.englishTitle() != show.title());
        CHECK(show.englishTitle() == "Money Heist");
    }

    SECTION("Loads the full cast for Stargate SG-1")
    {
        // Test everything, including cast.
        SettingsMock mockedSettings;
        TmdbTvConfiguration scraperConfig(mockedSettings);
        TmdbTv tmdb(scraperConfig);
        ShowScrapeJob::Config config{ShowIdentifier("4629"), Locale("en-US"), tmdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tmdbId() == TmdbId("4629"));
        REQUIRE(show.imdbId() == ImdbId("tt0118480"));

        // We used TMDB's `credits` field in the past. This field only contains
        // the actors of the _last_ season.  Newer MediaElch versions use
        // `aggregate_credits`, which also includes e.g. "Richard Dean Anderson".
        // There are only 6 actors listed for the last season.
        const auto& actors = show.actors().actors();
        // Yes, there are really 650 actors. Order from TMDb can change.
        REQUIRE(actors.size() > 650);
        const auto* anderson = [&actors]() -> const Actor* {
            for (const Actor* actor : actors) {
                if (actor != nullptr && actor->id == "26085") {
                    return actor;
                }
            }
            return nullptr;
        }();
        REQUIRE(anderson != nullptr);
        CHECK(anderson->name == "Richard Dean Anderson");
        CHECK(anderson->role == "Jack O'Neill");
        CHECK_THAT(anderson->thumb, StartsWith("https://image.tmdb.org/t/p/original"));

        test::scraper::compareAgainstReference(show, "scrapers/tmdbtv/Stargate-SG-1-tmdb4629");
    }
}
