#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "scrapers/tv_show/tvmaze/TvMazeShowScrapeJob.h"
#include "test/helpers/scraper_helpers.h"
#include "test/mocks/settings/SettingsMock.h"
#include "test/scrapers/tvmaze/testTvMazeHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("TvMaze scrapes show details", "[show][TvMaze][load_data]")
{
    SECTION("Loads minimal details for The Simpsons")
    {
        QSet<ShowScraperInfo> details{ShowScraperInfo::Title};
        ShowScrapeJob::Config config{ShowIdentifier("83"), Locale::English, details};

        auto scrapeJob = std::make_unique<TvMazeShowScrapeJob>(getTvMazeApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tvmazeId() == TvMazeId("83"));
        test::scraper::compareAgainstReference(show, "scrapers/tvmaze/The-Simpsons-minimal-details");
    }

    SECTION("Loads all details for The Simpsons")
    {
        SettingsMock mockSettings;
        TvMazeConfiguration scraperConfig(mockSettings);
        TvMaze tvdb(scraperConfig);

        ShowScrapeJob::Config config{ShowIdentifier("83"), Locale::English, tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TvMazeShowScrapeJob>(getTvMazeApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.imdbId() == ImdbId("tt0096697"));
        REQUIRE(show.tvdbId() == TvDbId("71663"));
        REQUIRE(show.tvmazeId() == TvMazeId("83"));
        test::scraper::compareAgainstReference(show, "scrapers/tvmaze/The-Simpsons-all-details");
    }

    SECTION("Loads all details for The Three Stooges")
    {
        SettingsMock mockSettings;
        TvMazeConfiguration scraperConfig(mockSettings);
        TvMaze tvdb(scraperConfig);

        ShowScrapeJob::Config config{ShowIdentifier("1751"), Locale::English, tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TvMazeShowScrapeJob>(getTvMazeApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tvdbId() == TvDbId("79173"));
        REQUIRE(show.tvmazeId() == TvMazeId("1751"));
        test::scraper::compareAgainstReference(show, "scrapers/tvmaze/The-Three-Stooges-all-details");
    }
}
