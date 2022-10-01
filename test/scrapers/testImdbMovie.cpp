#include "test/test_helpers.h"

#include "test/mocks/settings/MockScraperSettings.h"
#include "test/scrapers/testScraperHelpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/imdb/ImdbMovieSearchJob.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static ImdbApi& getImdbApi()
{
    static auto api = std::make_unique<ImdbApi>();
    return *api;
}

/// @brief Loads movie data synchronously
static void loadImdbSync(ImdbMovie& scraper, QHash<MovieScraper*, MovieIdentifier> ids, Movie& movie)
{
    const auto infos = scraper.meta().supportedDetails;
    QEventLoop loop;
    QEventLoop::connect(movie.controller(), &MovieController::sigInfoLoadDone, &loop, &QEventLoop::quit);
    scraper.loadData(ids, &movie, infos);
    loop.exec();
}

TEST_CASE("IMDb returns valid search results", "[IMDb][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Finding Dory", mediaelch::Locale::English};
        auto* searchJob = new ImdbMovieSearchJob(getImdbApi(), config);
        const auto scraperResults = searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].title == "Finding Dory");
        // Second result changes frequently but contains "Finding"
        CHECK(scraperResults[1].title.contains("Finding"));
    }

    SECTION("Search by IMDb ID returns correct results")
    {
        MovieSearchJob::Config config{"tt2277860", mediaelch::Locale::English};
        auto* searchJob = new ImdbMovieSearchJob(getImdbApi(), config);
        const auto scraperResults = searchMovieScraperSync(searchJob).first;

        // "Search" by ID actually loads the movie page, therefore only one result
        REQUIRE(scraperResults.length() == 1);
        CHECK_THAT(scraperResults[0].title, Matches("Finding Dory|Findet Dorie")); // Maintainer is German
        CHECK(scraperResults[0].released.toString("yyyy") == "2016");
    }
}

TEST_CASE("IMDb scrapes correct movie details", "[scraper][IMDb][load_data]")
{
    ImdbMovie imdb;
    MockScraperSettings settings(imdb.meta().identifier);
    settings.key_bool_map["LoadAllTags"] = false;
    imdb.loadSettings(settings);

    SECTION("'Normal' movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt2277860")}}, m);

        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        CHECK(m.tmdbId() == TmdbId::NoId);
        CHECK_THAT(m.name(), Matches("Finding Dory|Findet Dorie")); // Maintainer is German
        CHECK(m.originalName() == "Finding Dory");
        CHECK(m.certification() == Certification("PG"));
        CHECK(m.released().toString("yyyy-MM-dd") == "2016-09-29");
        // Finding Dory is rated 7.3 (date: 2018-08-31)
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().first().rating == Approx(7).margin(0.5));
        CHECK(m.ratings().first().voteCount > 6300);
        // Movie is not in top 250
        CHECK(m.top250() == 0);
        // Tagline may be different on each run, so we only
        // check if it is existent.
        CHECK_FALSE(m.tagline().isEmpty());
        CHECK(m.images().posters().size() == 1);
        CHECK(m.runtime() == 97min);

        CHECK_THAT(m.overview(), StartsWith("Dory is a wide-eyed, blue tang fish"));
        CHECK_THAT(m.outline(), StartsWith("Friendly but forgetful blue tang Dory"));
        CHECK_THAT(m.director(), Contains("Andrew Stanton"));
        CHECK_THAT(m.director(), Contains("Angus MacLane"));
        CHECK_THAT(m.writer(), Contains("Andrew Stanton"));
        CHECK_THAT(m.writer(), Contains("Victoria Strouse"));

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Animation");
        CHECK(genres[1] == "Adventure");

        const auto tags = m.tags();
        REQUIRE(tags.size() >= 2);
        CHECK(tags[0] == "father-son-relationship");
        CHECK(tags[1] == "no-opening-credits");

        const auto studios = m.studios();
        REQUIRE(studios.size() >= 2);
        CHECK(studios[0] == "Pixar Animation Studios");
        CHECK(studios[1] == "Walt Disney Pictures");

        const auto countries = m.countries();
        REQUIRE(countries.size() == 1);
        CHECK(countries[0] == "United States");

        const auto actors = m.actors().actors();
        REQUIRE(actors.size() >= 2);
        CHECK_THAT(actors, HasActor("Ellen DeGeneres", "Dory"));
        CHECK_THAT(actors, HasActor("Albert Brooks", "Marlin"));
    }

    SECTION("'Top 250' movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt0111161")}}, m);

        REQUIRE(m.imdbId() == ImdbId("tt0111161"));
        test::scraper::compareAgainstReference(m, "scrapers/imdb/The_Shawshank_Redemption_tt0111161");
    }

    SECTION("Loads tags correctly")
    {
        SECTION("'load all tags' is true")
        {
            settings.key_bool_map["LoadAllTags"] = true;
            imdb.loadSettings(settings);

            Movie m(QStringList{}); // Movie without files
            loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt0111161")}}, m);

            const auto tags = m.tags();
            REQUIRE(tags.size() >= 20);
            CHECK(tags[0] == "prison");
            CHECK_THAT(tags[1], Contains("stephen king"));
        }

        SECTION("'load all tags' is false")
        {
            settings.key_bool_map["LoadAllTags"] = false;
            imdb.loadSettings(settings);

            Movie m(QStringList{}); // Movie without files
            loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt0111161")}}, m);

            const auto tags = m.tags();
            REQUIRE(tags.size() >= 2);
            CHECK(tags[0] == "reading-lesson");
        }
    }

    SECTION("IMDb loads original title")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt2987732")}}, m);

        REQUIRE(m.imdbId() == ImdbId("tt2987732"));
        // translated english version / german original
        CHECK_THAT(m.name(), Matches("Suck Me Shakespeer|Fack ju Göhte"));
        if (m.name() == "Suck Me Shakespeer") {
            // original german title
            // Only appears if the site is the English version. If it's the German one,
            // no original title is shown.
            CHECK(m.originalName() == "Fack ju Göhte");
        }
    }

    SECTION("Movie with multiple countries is loaded")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt1663662")}}, m);

        REQUIRE(m.imdbId() == ImdbId("tt1663662"));
        test::scraper::compareAgainstReference(m, "scrapers/imdb/Pacific_Rim_tt1663662");
    }

    SECTION("Lesser known indian movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt3159708")}}, m);

        REQUIRE(m.imdbId() == ImdbId("tt3159708"));
        test::scraper::compareAgainstReference(m, "scrapers/imdb/Welcome_Back_tt3159708");
    }

    SECTION("Scraping movie two times does not increase actor count")
    {
        Movie m(QStringList{}); // Movie without files

        // load first time
        loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt2277860")}}, m);
        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        REQUIRE(m.actors().size() == 66);

        // load second time
        loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt2277860")}}, m);
        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        REQUIRE(m.actors().size() == 66);
    }

    SECTION("Godfather's Rating is loaded")
    {
        Movie m(QStringList{}); // Movie without files

        // The 2020-12 remake of IMDb's site has different rating layouts.
        // Godfather is one example.
        loadImdbSync(imdb, {{nullptr, MovieIdentifier("tt0068646")}}, m);
        REQUIRE(m.imdbId() == ImdbId("tt0068646"));
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().first().rating == Approx(9.2).margin(0.5));

        test::scraper::compareAgainstReference(m, "scrapers/imdb/Godfather_tt0068646");
    }
}
