#include "test/test_helpers.h"

#include "scrapers/KinoDe.h"

#include <chrono>

using namespace std::chrono_literals;

// KinoDe is a German website so search results and movie
// details in these tests are German as well.

TEST_CASE("KinoDe returns valid search results", "[scraper][KinoDe][search][requires_internet]")
{
    KinoDe KinoDe;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(KinoDe, "Findet Dorie");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Findet Dorie");
    }
}

TEST_CASE("KinoDe scrapes correct movie details", "[scraper][KinoDe][load_data][requires_internet]")
{
    KinoDe kinoDe;

    SECTION("'Normal' movie loaded by using IMDb id")
    {
        Movie m(QStringList{}); // Movie without files
        loadDataSync(kinoDe, {{nullptr, "findet-dorie-2016"}}, m, kinoDe.scraperNativelySupports());

        CHECK(m.imdbId() == ImdbId::NoId);
        CHECK(m.tmdbId() == TmdbId::NoId);

        CHECK(m.name() == "Findet Dorie");
        CHECK(m.originalName() == ""); // Not supported
        CHECK(m.certification() == "FSK 0");
        CHECK(m.released().toString("yyyy-MM-dd") == "2016-09-29");
        // TODO: Rating
        CHECK(m.runtime() == 97min);
        // There is only one main poster, but more than 10 backdrops
        CHECK(m.images().posters().size() == 1);
        CHECK(m.images().backdrops().size() >= 10);

        CHECK_THAT(m.overview(), StartsWith("Sechs Monate nach den Ereignissen aus „Findet Nemo“"));
        CHECK_THAT(m.outline(), StartsWith(R"(Findet Dorie: Im Sequel zum Pixar-Blockbuster "Findet Nemo")"));

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 3);
        CHECK(genres[0] == "Computeranimationsfilm");
        CHECK(genres[1] == "Animations- & Zeichentrickfilm");
        CHECK(genres[2] == "Familienfilm");

        const auto countries = m.countries();
        REQUIRE(countries.size() == 1);
        CHECK(countries[0] == "USA");
    }
}
