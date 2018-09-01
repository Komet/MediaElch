#include "test/test_helpers.h"

#include "scrapers/IMDB.h"

/**
 * @brief Loads movie data synchronously
 */
void loadImdbSync(IMDB &scraper, QMap<ScraperInterface *, QString> ids, Movie &movie)
{
    const auto infos = scraper.scraperSupports();
    QList<ScraperSearchResult> results;
    QEventLoop loop;
    // IMDb fires the "sigInfoLoadDone" event multiple times, e.g. when
    // details, tags and posters are loaded.
    // This may be fixed with future versions.
    uint32_t count = 0;
    loop.connect(movie.controller(), &MovieController::sigInfoLoadDone, [&]() {
        if (++count >= 2) {
            loop.quit();
        }
    });
    scraper.loadData(ids, &movie, infos);
    loop.exec();
}

TEST_CASE("IMDb returns valid search results", "[scraper][IMDb][search][requires_internet]")
{
    IMDB imdb;

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

TEST_CASE("IMDb scrapes correct movie details", "[scraper][IMDb][load_data][requires_internet]")
{
    IMDB imdb;

    SECTION("'Normal' movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, "tt2277860"}}, m);

        REQUIRE(m.id() == "tt2277860");
        CHECK(m.tmdbId() == "");
        CHECK(m.name() == "Finding Dory");
        CHECK(m.certification() == "PG");
        CHECK(m.released().toString("yyyy-MM-dd") == "2016-06-17");
        // Finding Dory is rated 7.3 (date: 2018-08-31)
        CHECK(m.rating() == Approx(7).margin(0.5));
        CHECK(m.votes() > 6300);
        // Movie is not in top 250
        CHECK(m.top250() == 0);
        // Tagline may be different on each run, so we only
        // check if it is existent.
        CHECK_FALSE(m.tagline().isEmpty());
        CHECK(m.images().posters().size() == 1);
        CHECK(m.runtime() == 97);

        CHECK_THAT(m.overview(), StartsWith("Dory is a wide-eyed, blue tang fish"));
        CHECK_THAT(m.outline(), StartsWith("The friendly but forgetful blue tang fish"));
        CHECK_THAT(m.director(), Contains("Andrew Stanton"));
        CHECK_THAT(m.writer(), Contains("Andrew Stanton"));

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Animation");
        CHECK(genres[1] == "Adventure");

        const auto tags = m.tags();
        REQUIRE(tags.size() >= 2);
        CHECK(tags[0] == "fish");
        CHECK(tags[1] == "ocean");

        const auto studios = m.studios();
        REQUIRE(studios.size() >= 2);
        CHECK(studios[0] == "Pixar Animation Studios");
        CHECK(studios[1] == "Walt Disney Pictures");

        const auto countries = m.countries();
        REQUIRE(countries.size() == 1);
        CHECK(countries[0] == "USA");

        const auto actors = m.actors();
        REQUIRE(actors.size() >= 2);
        CHECK(actors[0].name == "Ellen DeGeneres");
        CHECK(actors[0].role == "Dory");
        CHECK(actors[1].name == "Albert Brooks");
        CHECK(actors[1].role == "Marlin");
    }

    SECTION("'Top 250' movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadImdbSync(imdb, {{nullptr, "tt0111161"}}, m);

        REQUIRE(m.id() == "tt0111161");
        CHECK(m.name() == "The Shawshank Redemption");
        CHECK(m.certification() == "R");
        CHECK(m.released().toString("yyyy-MM-dd") == "1994-10-14");
        // "The Shawshank Redemption" is the highest rated IMDb movie
        CHECK(m.rating() == Approx(9.3).margin(0.5));
        CHECK(m.votes() > 6300);
        CHECK(m.top250() == 1);
        CHECK_FALSE(m.tagline().isEmpty());
        CHECK(m.images().posters().size() == 1);
        CHECK(m.runtime() == 142);

        CHECK_THAT(m.overview(), Contains("jailhouse of Shawshank"));
        CHECK_THAT(m.outline(), StartsWith("Two imprisoned men bond over a number of years"));
        CHECK_THAT(m.director(), Contains("Frank Darabont"));
        CHECK_THAT(m.director(), Contains("Stephen King"));
        CHECK_THAT(m.writer(), Contains("Stephen King"));
        CHECK_THAT(m.writer(), Contains("Tim Robbins"));

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 1);
        CHECK(genres[0] == "Drama");

        const auto tags = m.tags();
        REQUIRE(tags.size() >= 2);
        CHECK(tags[0] == "wrongful imprisonment");
        CHECK(tags[1] == "prison");

        const auto studios = m.studios();
        REQUIRE(studios.size() == 1);
        CHECK(studios[0] == "Castle Rock Entertainment");

        const auto countries = m.countries();
        REQUIRE(countries.size() >= 1);
        CHECK(countries[0] == "USA");

        const auto actors = m.actors();
        REQUIRE(actors.size() >= 2);
        CHECK(actors[0].name == "Tim Robbins");
        CHECK(actors[0].role == "Andy Dufresne");
        CHECK(actors[1].name == "Morgan Freeman");
        CHECK(actors[1].role == "Ellis Boyd 'Red' Redding");
    }
}
