#include "test/test_helpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "src/scrapers/tv_show/fernsehserien_de/FernsehserienDe.h"

#include "test/helpers/scraper_helpers.h"

using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("FernsehserienDe returns valid search results", "[FernsehserienDe][search]")
{
    auto api = std::make_unique<FernsehserienDeApi>();

    auto searchFor = [&api](const QString& query) -> QVector<ShowSearchJob::Result> {
        ShowSearchJob::Config config{query, Locale("de-DE")};
        auto* searchJob = new FernsehserienDeShowSearchJob(*api, config);
        return test::searchTvScraperSync(searchJob).first;
    };

    SECTION("Search by TV show name returns correct results")
    {
        const auto scraperResults = searchFor("Simpsons");

        REQUIRE(scraperResults.length() >= 4);
        CHECK(scraperResults[0].title == "Die Simpsons");
        CHECK(scraperResults[0].identifier.str() == "die-simpsons");
        CHECK(scraperResults[0].released == QDate(1989, 1, 1));

        // Special case for searching for "Simpsons": All results have a year.
        for (const auto& result : scraperResults) {
            CHECK(result.released.isValid());
            CHECK_FALSE(result.released.isNull());
        }
    }

    SECTION("Search by unique TV show that redirects to show page")
    {
        const auto scraperResults = searchFor("Scrubs");

        REQUIRE(scraperResults.length() == 1);
        CHECK(scraperResults[0].title == "Scrubs - Die Anfänger");
        CHECK(scraperResults[0].released == QDate(2001, 1, 1));
        CHECK(scraperResults[0].identifier.str() == "scrubs");
    }

    SECTION("Search by TV show with non-ASCII characters works: German")
    {
        // German kids show; "ö" must be properly encoded in the search query.
        const auto scraperResults = searchFor("Löwenzahn");

        REQUIRE(scraperResults.length() >= 5);
        CHECK(scraperResults[0].title == "Löwenzahn");
        CHECK(scraperResults[0].released == QDate(1981, 1, 1));
        CHECK(scraperResults[0].identifier.str() == "loewenzahn");
    }

    SECTION("Search by TV show with non-ASCII characters works: French")
    {
        // Some french TV show; I've never heard of it, but it has a few non-ASCII characters.
        const auto scraperResults = searchFor("Y’a pas d’âge");

        REQUIRE(scraperResults.length() >= 4);
        CHECK(scraperResults[0].title == "Y’a pas d’âge");
        CHECK(scraperResults[0].released == QDate(2013, 1, 1));
        CHECK(scraperResults[0].identifier.str() == "ya-pas-dage");
    }

    SECTION("Search by TV show with non-ASCII characters works: &")
    {
        // & is a special character in queries; it must be encoded
        const auto scraperResults = searchFor("G&G");

        REQUIRE(scraperResults.length() >= 40);
        CHECK(scraperResults[0].title == "G&G – Gesichter und Geschichten");
        CHECK(scraperResults[0].released == QDate(2005, 1, 1));
        CHECK(scraperResults[0].identifier.str() == "g-und-g-gesichter-und-geschichten");
    }

    SECTION("Search for unknown TV shows returns 0 results without an error")
    {
        ShowSearchJob::Config config{"SomethingThatDoesNotExist", Locale("de-DE")};
        auto* searchJob = new FernsehserienDeShowSearchJob(*api, config);
        const auto p = test::searchTvScraperSync(searchJob, true);

        CHECK(p.first.length() == 0);
        CHECK(p.second.error == ScraperError::Type::NoError);
    }
}
