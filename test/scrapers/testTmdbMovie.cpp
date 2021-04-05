#include "test/test_helpers.h"

#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovieSearchJob.h"
#include "settings/Settings.h"
#include "test/scrapers/testScraperHelpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static TmdbApi& getTmdbApi()
{
    static auto api = std::make_unique<TmdbApi>();
    if (!api->isInitialized()) {
        QEventLoop loop;
        QEventLoop::connect(api.get(), &TmdbApi::initialized, [&]() { loop.quit(); });
        api->initialize();
        loop.exec();
    }
    return *api;
}

TEST_CASE("TmdbMovie returns valid search results", "[TmdbMovie][search]")
{
    TmdbMovie tmdb;

    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Finding Dory", mediaelch::Locale::English};
        auto* searchJob = new TmdbMovieSearchJob(getTmdbApi(), config);
        const auto scraperResults = searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].title == "Finding Dory");
        CHECK(scraperResults[1].title == "Marine Life Interviews");
    }
}

TEST_CASE("TmdbMovie scrapes correct movie details", "[TmdbMovie][load_data]")
{
    TmdbMovie tmdb;
    Settings::instance()->setUsePlotForOutline(true);

    SECTION("'Normal' movie loaded by using IMDb id")
    {
        Movie m(QStringList{}); // Movie without files
        loadDataSync(tmdb, {{nullptr, MovieIdentifier("tt2277860")}}, m, tmdb.scraperNativelySupports());

        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        CHECK(m.tmdbId() == TmdbId("127380"));

        CHECK(m.name() == "Finding Dory");
        CHECK(m.originalName() == "Finding Dory");
        CHECK(m.certification() == Certification("PG"));
        CHECK(m.released().toString("yyyy-MM-dd") == "2016-06-16");
        // Finding Dory has a user score of 69% (date: 2018-08-31)
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().first().rating == Approx(6.9).margin(0.5));
        CHECK(m.ratings().first().voteCount > 6300);
        CHECK(m.tagline() == "An unforgettable journey she probably won't remember.");
        CHECK(m.runtime() == 97min);

        CHECK(m.set().tmdbId == TmdbId(137697));
        CHECK(m.set().name == "Finding Nemo Collection");
        CHECK_THAT(m.set().overview, StartsWithMatcher("A computer-animated adventure film series"));

        // https://www.youtube.com/watch?v=JhvrQeY3doI | may change from time to time
        CHECK_THAT(m.trailer().toString(), Contains("JhvrQeY3doI"));
        // There are more than 20 posters and backdrops
        // on TmdbMovie (using the API)
        CHECK(m.images().posters().size() >= 9);
        CHECK(m.images().backdrops().size() >= 13);

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

        const auto actors = m.actors().actors();
        REQUIRE(actors.size() >= 3);
        CHECK(actors[2]->name == "Ellen DeGeneres");
        CHECK(actors[2]->role == "Dory (voice)");
        CHECK(actors[1]->name == "Albert Brooks");
        CHECK(actors[1]->role == "Marlin (voice)");
    }

    SECTION("'Normal' movie loaded by using TmdbMovie id")
    {
        Movie m(QStringList{}); // Movie without files
        loadDataSync(tmdb, {{nullptr, MovieIdentifier("127380")}}, m, tmdb.scraperNativelySupports());

        REQUIRE(m.tmdbId() == TmdbId("127380"));
        CHECK(m.imdbId() == ImdbId("tt2277860"));
        CHECK(m.name() == "Finding Dory");

        // Rest is has already been tested and at this point we
        // can be sure that it's the same movie as above.
    }

    SECTION("Scraping movie two times does not increase actor count")
    {
        Movie m(QStringList{}); // Movie without files

        // load first time
        loadDataSync(tmdb, {{nullptr, MovieIdentifier("tt2277860")}}, m, tmdb.scraperNativelySupports());
        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        REQUIRE(m.actors().size() == 32);

        // load second time
        loadDataSync(tmdb, {{nullptr, MovieIdentifier("tt2277860")}}, m, tmdb.scraperNativelySupports());
        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        REQUIRE(m.actors().size() == 32);
    }
}
