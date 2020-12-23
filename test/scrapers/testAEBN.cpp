#include "test/test_helpers.h"

#include "scrapers/movie/aebn/AEBN.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

/// \brief Loads movie data synchronously
static void loadAebnMoviesSync(AEBN& scraper, QHash<MovieScraper*, QString> ids, Movie& movie)
{
    const auto infos = scraper.meta().supportedDetails;
    loadDataSync(scraper, ids, movie, infos);
}

TEST_CASE("AEBN returns valid search results", "[AEBN][search]")
{
    AEBN AEBN;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(AEBN, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Magic Mike XXXL: A Hardcore Parody");
    }
}

TEST_CASE("AEBN scrapes correct movie details", "[AEBN][load_data]")
{
    AEBN aebn;

    SECTION("Movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadAebnMoviesSync(aebn, {{nullptr, "188623"}}, m);

        REQUIRE_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        CHECK(m.imdbId() == ImdbId::NoId);
        CHECK(m.tmdbId() == TmdbId::NoId);
        CHECK(m.released().toString("yyyy") == "2015");
        CHECK(m.images().posters().size() == 1);
        CHECK(m.runtime() == 200min);

        CHECK_THAT(m.overview(), Contains("Magic Mike, Dallas, and the rest "));
        CHECK(m.director() == "Brad Armstrong");

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Feature");
        CHECK(genres[1] == "Adult Humor");

        const auto studios = m.studios();
        REQUIRE(!studios.empty());
        CHECK(studios[0] == "Wicked Pictures");

        const auto actors = m.actors();
        REQUIRE(actors.size() == 19);
        CHECK(actors[0]->name == "Misty Stone");
        CHECK(actors[1]->name == "Asa Akira");
    }

    SECTION("Movie has correct set")
    {
        Movie m(QStringList{}); // Movie without files
        loadAebnMoviesSync(aebn, {{nullptr, "159236"}}, m);
        CHECK(m.name() == "M Is For Mischief 3");
        CHECK(m.set().name == "M Is For Mischief");
    }
}
