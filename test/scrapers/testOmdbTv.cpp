#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/omdb/OmdbTv.h"
#include "scrapers/tv_show/omdb/OmdbTvConfiguration.h"
#include "scrapers/tv_show/omdb/OmdbTvEpisodeScrapeJob.h"
#include "scrapers/tv_show/omdb/OmdbTvSeasonScrapeJob.h"
#include "scrapers/tv_show/omdb/OmdbTvShowScrapeJob.h"
#include "scrapers/tv_show/omdb/OmdbTvShowSearchJob.h"
#include "settings/Settings.h"
#include "test/helpers/scraper_helpers.h"
#include "test/mocks/settings/SettingsMock.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static OmdbApi& getOmdbApi()
{
    static auto api = std::make_unique<OmdbApi>();
    if (!api->isInitialized()) {
        QString apiKey = QString::fromLocal8Bit(qgetenv("MEDIAELCH_OMDB_API_KEY"));
        api->setApiKey(apiKey);
        api->initialize();
    }
    return *api;
}

static bool hasOmdbApiKey()
{
    return !qgetenv("MEDIAELCH_OMDB_API_KEY").isEmpty();
}

TEST_CASE("OmdbTv returns valid search results", "[tv][OMDb][OmdbTv][search]")
{
    if (!hasOmdbApiKey()) {
        WARN("MEDIAELCH_OMDB_API_KEY not set — skipping OMDb TV tests");
        return;
    }

    SECTION("Search by show name returns correct results")
    {
        ShowSearchJob::Config config{"Breaking Bad", mediaelch::Locale::English};
        auto* searchJob = new OmdbTvShowSearchJob(getOmdbApi(), config);
        const auto scraperResults = test::searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Breaking Bad");
    }

    SECTION("Search by IMDB ID returns single correct result")
    {
        ShowSearchJob::Config config{"tt0903747", mediaelch::Locale::English};
        auto* searchJob = new OmdbTvShowSearchJob(getOmdbApi(), config);
        const auto scraperResults = test::searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() == 1);
        CHECK(scraperResults[0].title == "Breaking Bad");
        CHECK(scraperResults[0].identifier.str() == "tt0903747");
    }
}

TEST_CASE("OmdbTv scrapes correct show details", "[tv][OMDb][OmdbTv][load_data]")
{
    if (!hasOmdbApiKey()) {
        WARN("MEDIAELCH_OMDB_API_KEY not set — skipping OMDb TV tests");
        return;
    }

    SECTION("Show loaded by IMDB ID has expected fields")
    {
        // Breaking Bad
        ShowScrapeJob::Config config{
            ShowIdentifier("tt0903747"), mediaelch::Locale::English, mediaelch::allShowScraperInfos()};
        auto scrapeJob = std::make_unique<OmdbTvShowScrapeJob>(getOmdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.title() == "Breaking Bad");
        CHECK(show.firstAired().year() == 2008);
        CHECK(show.certification().toString() == "TV-MA");
        CHECK(show.genres().size() >= 1);
        CHECK_THAT(show.overview(), Contains("Walter White"));
        CHECK(show.ratings().hasSource("imdb"));
    }
}
