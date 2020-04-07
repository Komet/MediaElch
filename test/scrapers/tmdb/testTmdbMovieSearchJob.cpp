#include "test/test_helpers.h"

#include "scrapers/movie/tmdb/TmdbMovie.h"

#include "test/scrapers/testScraperHelper.h"

#include <QRegularExpression>
#include <QSignalSpy>
#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;

TEST_CASE("TMDb Movie: search movies", "[scraper][TMDb][search][requires_internet]")
{
    mediaelch::scraper::TmdbMovie tmdb;

    QSignalSpy spy(&tmdb, &mediaelch::scraper::TmdbMovie::initialized);
    tmdb.initialize();
    spy.wait(3000);

    REQUIRE(tmdb.isInitialized());

    SECTION("Search for movie with English title")
    {
        scraper::MovieSearchJob::Config config("Star Wars", scraper::Locale("en-US"));
        auto results = test::searchMovieSync(tmdb, config);

        // Default: Load 3 pages, i.e. 60 entries.
        CHECK(results.size() == 60);

        for (const auto& result : results) {
            CAPTURE(result.title);
            CHECK_THAT(result.title.toLower(), Contains("star"));
            CHECK_THAT(result.title.toLower(), Contains("wars"));
            CHECK_THAT(result.identifier, Matches(R"(\d+)"));
            if (result.released.isValid()) {
                CHECK(result.released.year() >= 1977); // 1977 => first Star Wars movie
            }
        }
    }

    SECTION("Search for movie with German title")
    {
        scraper::MovieSearchJob::Config config("Star Wars", scraper::Locale("de-DE"));
        auto results = test::searchMovieSync(tmdb, config);

        // Default: Load 3 pages, i.e. 60 entries.
        CHECK(results.size() == 60);

        bool foundGerman = false;
        SECTION("Results must all be about Star Wars")
        {
            for (const auto& result : results) {
                CAPTURE(result.title);
                CHECK_THAT(result.title.toLower(), Matches("sterne|star"));
                CHECK_THAT(result.title.toLower(), Matches("krieg|wars"));
                CHECK_THAT(result.identifier, Matches(R"(\d+)"));
                if (result.released.isValid()) {
                    CHECK(result.released.year() >= 1977); // 1977 => first Star Wars movie
                }
                foundGerman = foundGerman | result.title.startsWith("Krieg der Sterne");
            }

            SECTION("There must be at least one German movie if de-DE was requested") { CHECK(foundGerman); }
        }
    }
}
