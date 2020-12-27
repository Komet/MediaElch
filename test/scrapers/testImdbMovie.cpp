#include "test/test_helpers.h"

#include "test/mocks/settings/MockScraperSettings.h"

#include "scrapers/movie/imdb/ImdbMovie.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

/// @brief Loads movie data synchronously
static void loadImdbSync(ImdbMovie& scraper, QHash<MovieScraper*, QString> ids, Movie& movie)
{
    const auto infos = scraper.meta().supportedDetails;
    QEventLoop loop;
    QEventLoop::connect(movie.controller(), &MovieController::sigInfoLoadDone, [&]() { loop.quit(); });
    scraper.loadData(ids, &movie, infos);
    loop.exec();
}

TEST_CASE("IMDb returns valid search results", "[IMDb][search]")
{
    ImdbMovie imdb;
    MockScraperSettings settings(imdb.meta().identifier);
    imdb.loadSettings(settings);

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(imdb, "Finding Dory");
        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].name == "Finding Dory");
        // Second result changes frequently but contains "Finding"
        CHECK(scraperResults[1].name.contains("Finding"));
    }

    SECTION("Search by IMDb ID returns correct results")
    {
        const auto scraperResults = searchScraperSync(imdb, "tt2277860");
        // "Search" by ID actually loads the movie page, therefore only one result
        REQUIRE(scraperResults.length() == 1);
        REQUIRE(scraperResults[0].name == "Finding Dory");
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
        loadImdbSync(imdb, {{nullptr, "tt2277860"}}, m);

        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        CHECK(m.tmdbId() == TmdbId::NoId);
        CHECK(m.name() == "Finding Dory");
        CHECK(m.certification() == Certification("PG"));
        CHECK(m.released().toString("yyyy-MM-dd") == "2016-06-17");
        // Finding Dory is rated 7.3 (date: 2018-08-31)
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().back().rating == Approx(7).margin(0.5));
        CHECK(m.ratings().back().voteCount > 6300);
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
        CHECK_THAT(m.writer(), Contains("Andrew Stanton"));

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Animation");
        CHECK(genres[1] == "Adventure");

        const auto tags = m.tags();
        REQUIRE(tags.size() >= 2);
        CHECK(tags[0] == "father son relationship");
        CHECK(tags[1] == "no opening credits");

        const auto studios = m.studios();
        REQUIRE(studios.size() >= 2);
        CHECK(studios[0] == "Pixar Animation Studios");
        CHECK(studios[1] == "Walt Disney Pictures");

        const auto countries = m.countries();
        REQUIRE(countries.size() == 1);
        CHECK(countries[0] == "USA");

        const auto actors = m.actors();
        REQUIRE(actors.size() >= 2);
        CHECK(actors[0]->name == "Ellen DeGeneres");
        CHECK(actors[0]->role == "Dory");
        CHECK(actors[1]->name == "Albert Brooks");
        CHECK(actors[1]->role == "Marlin");
    }

    SECTION("'Top 250' movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, "tt0111161"}}, m);

        REQUIRE(m.imdbId() == ImdbId("tt0111161"));
        CHECK(m.name() == "The Shawshank Redemption");
        CHECK(m.certification() == Certification("R"));
        CHECK(m.released().toString("yyyy-MM-dd") == "1994-10-14");
        // "The Shawshank Redemption" is the highest rated IMDb movie
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().back().rating == Approx(9.3).margin(0.5));
        CHECK(m.ratings().back().voteCount > 6300);
        CHECK(m.top250() == 1);
        // Tagline may be different on each run, so we only
        // check if it is existent.
        CHECK_FALSE(m.tagline().isEmpty());
        CHECK(m.images().posters().size() == 1);
        CHECK(m.runtime() == 142min);

        CHECK_THAT(m.overview(), Contains("jailhouse of Shawshank"));
        CHECK_THAT(m.outline(), StartsWith("Two imprisoned men bond over a number of years"));
        CHECK_THAT(m.director(), Contains("Frank Darabont"));
        CHECK_THAT(m.director(), ContainsNot("Stephen King")); // is actually a writer
        CHECK_THAT(m.writer(), Contains("Stephen King"));
        CHECK_THAT(m.writer(), Contains("Frank Darabont"));
        CHECK_THAT(m.writer(), ContainsNot("Tim Robbins")); // is actually a star

        const auto genres = m.genres();
        REQUIRE(!genres.empty());
        CHECK(genres[0] == "Drama");

        const auto tags = m.tags();
        REQUIRE(tags.size() >= 2);
        CHECK(tags[0] == "wrongful imprisonment");
        CHECK_THAT(tags[1], Contains("stephen king"));

        const auto studios = m.studios();
        REQUIRE(studios.size() == 1);
        CHECK(studios[0] == "Castle Rock Entertainment");

        const auto countries = m.countries();
        REQUIRE(!countries.empty());
        CHECK(countries[0] == "USA");

        const auto actors = m.actors();
        REQUIRE(actors.size() >= 2);
        CHECK(actors[0]->name == "Tim Robbins");
        CHECK(actors[0]->role == "Andy Dufresne");
        CHECK(actors[1]->name == "Morgan Freeman");
        CHECK(actors[1]->role == "Ellis Boyd 'Red' Redding");
    }

    SECTION("Loads tags correctly")
    {
        SECTION("'load all tags' is true")
        {
            settings.key_bool_map["LoadAllTags"] = true;
            imdb.loadSettings(settings);

            Movie m(QStringList{}); // Movie without files
            loadImdbSync(imdb, {{nullptr, "tt0111161"}}, m);

            const auto tags = m.tags();
            REQUIRE(tags.size() >= 20);
            CHECK(tags[0] == "wrongful imprisonment");
            CHECK_THAT(tags[1], Contains("stephen king"));
        }

        SECTION("'load all tags' is false")
        {
            settings.key_bool_map["LoadAllTags"] = false;
            imdb.loadSettings(settings);

            Movie m(QStringList{}); // Movie without files
            loadImdbSync(imdb, {{nullptr, "tt0111161"}}, m);

            const auto tags = m.tags();
            REQUIRE(tags.size() >= 2);
            CHECK(tags[0] == "wrongful imprisonment");
            CHECK_THAT(tags[1], Contains("stephen king"));
        }
    }

    SECTION("IMDb loads original title")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, "tt2987732"}}, m);

        REQUIRE(m.imdbId() == ImdbId("tt2987732"));
        CHECK(m.name() == "Suck Me Shakespeer");    // translated english version
        CHECK(m.originalName() == "Fack ju Göhte"); // original german title
    }

    SECTION("Lesser known indian movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, "tt3159708"}}, m);

        REQUIRE(m.imdbId() == ImdbId("tt3159708"));
        CHECK(m.name() == "Welcome Back");
        CHECK(m.certification() == Certification("Not Rated"));
        CHECK(m.released().toString("yyyy-MM-dd") == "2015-09-04");
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().back().rating == Approx(4.2).margin(0.5));
        CHECK(m.ratings().back().voteCount > 4800);
        CHECK_FALSE(m.images().posters().isEmpty());
        CHECK(m.runtime() == 152min);

        CHECK_THAT(m.overview(), Contains("have left the underworld"));
        CHECK_THAT(m.outline(), StartsWith("A pair of reformed gangsters try to find"));
        CHECK_THAT(m.director(), Contains("Anees Bazmee"));
        CHECK_THAT(m.writer(), Contains("Rajeev Kaul"));

        const auto genres = m.genres();
        REQUIRE(!genres.empty());
        CHECK(genres[0] == "Action");

        const auto studios = m.studios();
        REQUIRE(studios.size() == 1);
        CHECK(studios[0] == "Base Industries Group");

        const auto countries = m.countries();
        REQUIRE(!countries.empty());
        CHECK(countries[0] == "India");

        const auto actors = m.actors();
        REQUIRE(actors.size() >= 5);
        CHECK(actors[0]->name == "Anil Kapoor");
        CHECK(actors[0]->role == "Sagar 'Majnu' Pandey");
        CHECK(actors[1]->name == "Nana Patekar");
        CHECK(actors[1]->role == "Uday Shankar Shetty");
        CHECK(actors[2]->name == "Dimple Kapadia");
        CHECK_THAT(actors[2]->role, Contains("Poonam"));
        CHECK(actors[3]->name == "John Abraham");
        CHECK_THAT(actors[3]->role, Contains("Ajju Bhai"));
        CHECK(actors[4]->name == "Shruti Haasan");
        CHECK_THAT(actors[4]->role, Contains("Ranjana Shetty"));
    }

    SECTION("Scraping movie two times does not increase actor count")
    {
        Movie m(QStringList{}); // Movie without files

        // load first time
        loadImdbSync(imdb, {{nullptr, "tt2277860"}}, m);
        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        REQUIRE(m.actors().size() == 15);

        // load second time
        loadImdbSync(imdb, {{nullptr, "tt2277860"}}, m);
        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        REQUIRE(m.actors().size() == 15);
    }
}
