#include "test/test_helpers.h"

#include "scrapers/HotMovies.h"

#include <chrono>

using namespace std::chrono_literals;

/**
 * @brief Loads movie data synchronously
 */
void loadHotMoviesSync(HotMovies &scraper, QMap<MovieScraperInterface *, QString> ids, Movie &movie)
{
    const auto infos = scraper.scraperSupports();
    QList<ScraperSearchResult> results;
    QEventLoop loop;
    loop.connect(movie.controller(), &MovieController::sigInfoLoadDone, [&]() { loop.quit(); });
    scraper.loadData(ids, &movie, infos);
    loop.exec();
}

TEST_CASE("HotMovies returns valid search results", "[scraper][HotMovies][search][requires_internet]")
{
    HotMovies HotMovies;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(HotMovies, "Magic Mike XXXL");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Magic Mike XXXL: A Hardcore Parody");
    }
}


TEST_CASE("HotMovies scrapes correct movie details", "[scraper][HotMovies][load_data][requires_internet]")
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
        // Rating currently no available
        // CHECK(m.rating() == Approx(4).margin(0.5));
        CHECK(m.votes() > 60);
        CHECK(m.images().posters().size() == 1);
        CHECK(m.runtime() == 201min);

        CHECK_THAT(m.overview(), Contains("with a great storyline"));
        CHECK(m.director() == "Brad Armstrong");

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Contemporary");
        CHECK(genres[1] == "Spoofs / Parodies");

        const auto studios = m.studios();
        REQUIRE(studios.size() >= 1);
        CHECK(studios[0] == "Wicked Pictures");

        const auto actors = m.actors();
        REQUIRE(actors.size() > 15);
        CHECK(actors[0].name == "Adriana Chechik");
        CHECK(actors[1].name == "Amirah Adara");
    }

    SECTION("Movie has correct set")
    {
        Movie m(QStringList{}); // Movie without files
        loadHotMoviesSync(hm, {{nullptr, "https://www.hotmovies.com/video/214343/-M-Is-For-Mischief-Number-3/"}}, m);
        REQUIRE(m.name() == "\"M\" Is For Mischief Number 3");
        REQUIRE(m.set() == "\"M\" Is For Mischief");
    }
}
