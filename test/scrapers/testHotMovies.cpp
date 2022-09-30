#include "test/test_helpers.h"

#include "scrapers/movie/hotmovies/HotMovies.h"
#include "scrapers/movie/hotmovies/HotMoviesSearchJob.h"
#include "test/scrapers/testScraperHelpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static HotMoviesApi& getHotMoviesApi()
{
    static auto api = std::make_unique<HotMoviesApi>();
    return *api;
}

/// @brief Loads movie data synchronously
static void loadHotMoviesSync(HotMovies& scraper, QHash<MovieScraper*, MovieIdentifier> ids, Movie& movie)
{
    const auto infos = scraper.meta().supportedDetails;
    loadDataSync(scraper, ids, movie, infos);
}

TEST_CASE("HotMovies returns valid search results", "[HotMovies][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Magic Mike XXXL", mediaelch::Locale::English};
        auto* searchJob = new HotMoviesSearchJob(getHotMoviesApi(), config);
        const auto scraperResults = searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Magic Mike XXXL: A Hardcore Parody");
    }
}


TEST_CASE("HotMovies scrapes correct movie details", "[HotMovies][load_data]")
{
    HotMovies hm;

    SECTION("Movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadHotMoviesSync(hm,
            {{nullptr, MovieIdentifier("https://www.hotmovies.com/video/292788/Magic-Mike-XXXL-A-Hardcore-Parody/")}},
            m);

        REQUIRE_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        test::compareAgainstReference(m, "scrapers/hot-movies/Magic-Mike-292788");
    }

    SECTION("Movie has correct set")
    {
        Movie m(QStringList{}); // Movie without files
        loadHotMoviesSync(
            hm, {{nullptr, MovieIdentifier("https://www.hotmovies.com/video/214343/-M-Is-For-Mischief-Number-3/")}}, m);

        REQUIRE(m.name() == "\"M\" Is For Mischief Number 3");
        REQUIRE(m.set().name == "\"M\" Is For Mischief");
        test::compareAgainstReference(m, "scrapers/hot-movies/M-Is-For-Mischief-214343");
    }
}
