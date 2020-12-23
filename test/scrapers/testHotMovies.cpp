#include "test/test_helpers.h"

#include "scrapers/movie/hotmovies/HotMovies.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

/// @brief Loads movie data synchronously
static void loadHotMoviesSync(HotMovies& scraper, QHash<MovieScraper*, QString> ids, Movie& movie)
{
    const auto infos = scraper.meta().supportedDetails;
    loadDataSync(scraper, ids, movie, infos);
}

TEST_CASE("HotMovies returns valid search results", "[HotMovies][search]")
{
    HotMovies HotMovies;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(HotMovies, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Magic Mike XXXL: A Hardcore Parody");
    }
}


TEST_CASE("HotMovies scrapes correct movie details", "[HotMovies][load_data]")
{
    HotMovies hm;

    SECTION("Movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadHotMoviesSync(
            hm, {{nullptr, "https://www.hotmovies.com/video/292788/Magic-Mike-XXXL-A-Hardcore-Parody/"}}, m);

        REQUIRE_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        CHECK(m.imdbId() == ImdbId::NoId);
        CHECK(m.tmdbId() == TmdbId::NoId);
        CHECK(m.released().toString("yyyy") == "2015");
        REQUIRE(!m.ratings().isEmpty());
        // Rating currently no available
        // CHECK(m.ratings().back().rating == Approx(4).margin(0.5));
        CHECK(m.ratings().back().voteCount > 50);
        CHECK(m.images().posters().size() == 1);
        CHECK(m.images().backdrops().size() == 1);
        CHECK(m.runtime() == 201min);

        CHECK_THAT(m.overview(), Contains("with a great storyline"));
        CHECK(m.director() == "Brad Armstrong");

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Contemporary");
        CHECK(genres[1] == "Spoofs / Parodies");

        const auto studios = m.studios();
        REQUIRE(!studios.empty());
        CHECK(studios[0] == "Wicked Pictures");

        const auto actors = m.actors();
        REQUIRE(actors.size() > 15);
        CHECK(actors[0]->name == "Adriana Chechik");
        CHECK(actors[0]->thumb == "https://img2.vod.com/image2/star/163/Adriana_Chechik-163576.4.jpg");
        CHECK(actors[1]->name == "Amirah Adara");
        CHECK_THAT(actors[1]->thumb, StartsWith("https://"));
        CHECK_THAT(actors[1]->thumb, Contains("Amirah_Adara"));
        CHECK_THAT(actors[1]->thumb, EndsWith(".jpg"));
    }

    SECTION("Movie has correct set")
    {
        Movie m(QStringList{}); // Movie without files
        loadHotMoviesSync(hm, {{nullptr, "https://www.hotmovies.com/video/214343/-M-Is-For-Mischief-Number-3/"}}, m);
        CHECK(m.name() == "\"M\" Is For Mischief Number 3");
        CHECK(m.set().name == "\"M\" Is For Mischief");
    }
}
