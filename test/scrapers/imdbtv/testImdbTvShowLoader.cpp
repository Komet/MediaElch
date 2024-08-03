#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h"
#include "test/helpers/scraper_helpers.h"
#include "test/mocks/settings/SettingsMock.h"
#include "test/scrapers/imdbtv/testImdbTvHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("ImdbTv scrapes show details", "[show][ImdbTv][load_data]")
{
    SECTION("Loads minimal details for The Simpsons")
    {
        QSet<ShowScraperInfo> details{ShowScraperInfo::Title};
        ShowScrapeJob::Config config{ShowIdentifier("tt0096697"), Locale("en-US"), details};

        auto scrapeJob = std::make_unique<ImdbTvShowScrapeJob>(getImdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.imdbId() == ImdbId("tt0096697"));
        test::scraper::compareAgainstReference(show, "scrapers/imdbtv/The-Simpsons-tt0096697-minimal-details");
    }

    SECTION("Loads all details for Scrubs")
    {
        SettingsMock mockSettings;
        ImdbTvConfiguration scraperConfig(mockSettings);
        ImdbTv imdbTv(scraperConfig);
        ShowScrapeJob::Config config{ShowIdentifier("tt0285403"), Locale("en-US"), imdbTv.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<ImdbTvShowScrapeJob>(getImdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.imdbId() == ImdbId("tt0285403"));
        test::scraper::compareAgainstReference(show, "scrapers/imdbtv/Scrubs-tt0285403");
    }

    SECTION("Loads all details for a TV series with HTML entities")
    {
        SettingsMock mockSettings;
        ImdbTvConfiguration scraperConfig(mockSettings);
        ImdbTv imdbTv(scraperConfig);
        ShowScrapeJob::Config config{ShowIdentifier("tt1384816"), Locale("en-US"), imdbTv.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<ImdbTvShowScrapeJob>(getImdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.imdbId() == ImdbId("tt1384816"));
        test::scraper::compareAgainstReference(show, "scrapers/imdbtv/Maters-Tall-Tales-tt1384816");
    }


    SECTION("Loads correct runtime for Sherlock (2010)")
    {
        // Note: Sherlock runs longer than 1h
        //       This test ensures that the runtime is correctly scraped.

        SettingsMock mockSettings;
        ImdbTvConfiguration scraperConfig(mockSettings);
        ImdbTv imdbTv(scraperConfig);
        ShowScrapeJob::Config config{ShowIdentifier("tt1475582"), Locale("en-US"), imdbTv.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<ImdbTvShowScrapeJob>(getImdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.imdbId() == ImdbId("tt1475582"));
        test::scraper::compareAgainstReference(show, "scrapers/imdbtv/Sherlock-tt0285403");
        CHECK(show.runtime() == 90min);
    }

    // Other languages are not yet supported. Reason:
    // IMDb filters based on IP. Only if users have an account, changing the language is possible.
    //
    // SECTION("Loads all details for Scrubs in another Language")
    // {
    //     SettingsMock mockSettings;
    //     ImdbTvConfiguration scraperConfig(mockSettings);
    //     ImdbTv imdbTv(scraperConfig);
    //     ShowScrapeJob::Config config{ShowIdentifier("tt0285403"), Locale("de-DE"),
    //     imdbTv.meta().supportedShowDetails};

    //     auto scrapeJob = std::make_unique<ImdbTvShowScrapeJob>(getImdbApi(), config);
    //     test::scrapeTvScraperSync(scrapeJob.get());
    //     auto& show = scrapeJob->tvShow();

    //     CHECK(show.imdbId() == ImdbId("tt0285403"));

    //     CHECK(show.title() == "Scrubs");
    //     CHECK(show.certification() == Certification("TV-14"));
    //     CHECK(show.firstAired() == QDate(2001, 10, 2));
    //     CHECK_THAT(show.overview(),
    //         StartsWith("Mittelpunkt und Ich-Erzähler der Serie ist der junge Mediziner John Michael Dorian"));

    //     REQUIRE_FALSE(show.ratings().isEmpty());
    //     CHECK(show.ratings().first().rating == Approx(8.3).margin(0.5));
    //     CHECK(show.ratings().first().voteCount > 3000);

    //     CHECK(show.runtime() == 22min);

    //     const auto& genres = show.genres();
    //     REQUIRE(genres.size() > 0);
    //     CHECK_THAT(genres[0], Contains("Comedy"));
    // }
}
