#include "test/test_helpers.h"

#include "scrapers/TMDb.h"

TEST_CASE("TMDb returns valid search results", "[scraper][TMDb][search][requires_internet]")
{
    TMDb TMDb;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(TMDb, "Finding Dory");
        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].name == "Finding Dory");
        CHECK(scraperResults[1].name == "Marine Life Interviews");
    }
}

TEST_CASE("TMDb scrapes correct movie details", "[scraper][TMDb][load_data][requires_internet]")
{
    TMDb tmdb;

    SECTION("'Normal' movie loaded by using IMDb id")
    {
        Movie m(QStringList{}); // Movie without files
        loadDataSync(tmdb, {{nullptr, "tt2277860"}}, m, tmdb.scraperNativelySupports());

        REQUIRE(m.id() == "tt2277860");
        CHECK(m.tmdbId() == "127380");

        CHECK(m.name() == "Finding Dory");
        CHECK(m.originalName() == "Finding Dory");
        CHECK(m.certification() == "PG");
        CHECK(m.released().toString("yyyy-MM-dd") == "2016-06-16");
        // Finding Dory has a user score of 69% (date: 2018-08-31)
        CHECK(m.rating() == Approx(6.9).margin(0.5));
        CHECK(m.votes() > 6300);
        CHECK(m.tagline() == "An unforgettable journey she probably won't remember.");
        CHECK(m.runtime() == 97);
        CHECK(m.set() == "Finding Nemo Collection");
        CHECK(m.trailer() == QUrl("plugin://plugin.video.youtube/?action=play_video&videoid=JhvrQeY3doI"));
        // There are more than 20 posters and backdrops
        // on TMDb (using the API)
        CHECK(m.images().posters().size() >= 20);
        CHECK(m.images().backdrops().size() >= 20);

        CHECK_THAT(m.overview(), StartsWith("Dory is reunited with her friends Nemo and Marlin"));
        CHECK_THAT(m.outline(), StartsWith("Dory is reunited with her friends Nemo and Marlin"));
        CHECK(m.director() == "Andrew Stanton");
        CHECK_THAT(m.writer(), Contains("Andrew Stanton"));
        CHECK_THAT(m.writer(), Contains("Victoria Strouse"));

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 3);
        CHECK(genres[0] == "Adventure");
        CHECK(genres[1] == "Animation");
        CHECK(genres[2] == "Comedy");

        const auto studios = m.studios();
        REQUIRE(studios.size() == 1);
        CHECK(studios[0] == "Pixar");

        const auto countries = m.countries();
        REQUIRE(countries.size() == 1);
        CHECK(countries[0] == "United States of America");

        const auto actors = m.actors();
        REQUIRE(actors.size() >= 2);
        CHECK(actors[0].name == "Ellen DeGeneres");
        CHECK(actors[0].role == "Dory (voice)");
        CHECK(actors[1].name == "Albert Brooks");
        CHECK(actors[1].role == "Marlin (voice)");
    }


    SECTION("'Normal' movie loaded by using TMDb id")
    {
        Movie m(QStringList{}); // Movie without files
        loadDataSync(tmdb, {{nullptr, "127380"}}, m, tmdb.scraperNativelySupports());

        REQUIRE(m.tmdbId() == "127380");
        CHECK(m.id() == "tt2277860");
        CHECK(m.name() == "Finding Dory");

        // Rest is has already been tested and at this point we
        // can be sure that it's the same movie as above.
    }
}
