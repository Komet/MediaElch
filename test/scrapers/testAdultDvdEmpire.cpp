#include "test/test_helpers.h"

#include "scrapers/movie/AdultDvdEmpire.h"

#include <chrono>

using namespace std::chrono_literals;

/// @brief Loads movie data synchronously
void loadAdultDvdEmpireSync(AdultDvdEmpire& scraper, QMap<MovieScraperInterface*, QString> ids, Movie& movie)
{
    const auto infos = scraper.scraperSupports();
    loadDataSync(scraper, ids, movie, infos);
}

TEST_CASE("AdultDvdEmpire returns valid search results", "[scraper][AdultDvdEmpire][search][requires_internet]")
{
    AdultDvdEmpire adultDvdEmpire;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(adultDvdEmpire, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() == 2);
        // one for DVDs and one for VOD
        CHECK(scraperResults[0].name == "[DVD] Magic Mike XXXL");
        CHECK(scraperResults[1].name == "[VOD] Magic Mike XXXL");
    }
}

TEST_CASE("AdultDvdEmpire scrapes correct movie details", "[scraper][AdultDvdEmpire][load_data][requires_internet]")
{
    AdultDvdEmpire hm;

    SECTION("Movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadAdultDvdEmpireSync(hm, {{nullptr, "/1745335/magic-mike-xxxl-porn-movies.html"}}, m);

        CHECK_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        CHECK(m.imdbId() == ImdbId::NoId);
        CHECK(m.tmdbId() == TmdbId::NoId);
        CHECK(m.released().toString("yyyy") == "2015");

        CHECK(m.images().posters().size() == 1);
        CHECK(m.images().backdrops().size() > 70);
        CHECK(m.runtime() == 201min);

        CHECK_THAT(m.overview(), Contains("Award-Winning Director Brad Armstrong brings you"));
        CHECK(m.director() == "Brad Armstrong");

        const auto genres = m.genres();
        REQUIRE(genres.size() == 10);
        CHECK(genres[0] == "Big Budget");
        CHECK_THAT(genres[1], StartsWith("Big"));

        const auto studios = m.studios();
        REQUIRE(!studios.empty());
        CHECK(studios[0] == "Wicked Pictures");

        const auto actors = m.actors();
        REQUIRE(actors.size() > 15);
        bool foundActor = false;
        for (const auto& actor : actors) {
            foundActor = foundActor
                         || (actor->name == "Adriana Chechik"
                             && actor->thumb == "https://imgs1cdn.adultempire.com/actors/652646h.jpg");
        }
        CHECK(foundActor);
    }

    SECTION("Movie has correct set")
    {
        Movie m(QStringList{}); // Movie without files
        loadAdultDvdEmpireSync(hm, {{nullptr, "/1613899/m-is-for-mischief-no-3-porn-movies.html"}}, m);
        CHECK(m.name() == "\"M\" Is For Mischief No. 3");
        CHECK(m.set().name == "\"M\" Is For Mischief");
    }
}
