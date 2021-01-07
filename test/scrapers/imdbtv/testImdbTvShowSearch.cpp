#include "test/test_helpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "src/scrapers/tv_show/imdb/ImdbTvShowSearchJob.h"
#include "test/scrapers/imdbtv/testImdbTvHelper.h"
#include "test/scrapers/testScraperHelpers.h"

using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("ImdbTv returns valid search results", "[tv][ImdbTv][search]")
{
    SECTION("Search by TV show name returns correct results")
    {
        ShowSearchJob::Config config{"The Simpsons", Locale::English};
        auto* searchJob = new ImdbTvShowSearchJob(getImdbApi(), config);
        const auto scraperResults = searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 10);
        CHECK(scraperResults[0].title == "The Simpsons");
        CHECK(scraperResults[0].identifier.str() == "tt0096697");
        CHECK(scraperResults[0].released == QDate(1989, 1, 1)); // only year is set
    }

    SECTION("Search by TV show name in other languages returns correct results")
    {
        ShowSearchJob::Config config{"Scrubs", Locale("de-DE")};
        auto* searchJob = new ImdbTvShowSearchJob(getImdbApi(), config);
        const auto scraperResults = searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 3);
        CHECK(scraperResults[0].title == "Scrubs: Die Anfänger");
        CHECK(scraperResults[0].identifier.str() == "tt0285403");
        CHECK(scraperResults[0].released == QDate(2001, 1, 1)); // only year is set
    }

    SECTION("Search by TV show name returns 0 results for unknown shows")
    {
        ShowSearchJob::Config config{"SomethingThatDoesNotExist", Locale::English};
        auto* searchJob = new ImdbTvShowSearchJob(getImdbApi(), config);
        const auto p = searchTvScraperSync(searchJob, true);

        CHECK(p.first.length() == 0);
        CHECK(p.second.error == ScraperError::Type::NoError);
    }
}
