#include "test/test_helpers.h"

#include "scrapers/movie/videobuster/VideoBuster.h"
#include "settings/Settings.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

// VideoBuster is a German website so search results and movie
// details in these tests are German as well.

TEST_CASE("VideoBuster returns valid search results", "[VideoBuster][search]")
{
    VideoBuster VideoBuster;

    SECTION("Search by movie name returns correct results")
    {
        const auto scraperResults = searchScraperSync(VideoBuster, "Findet Dorie");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Findet Dorie");
    }
}


TEST_CASE("VideoBuster scrapes correct movie details", "[VideoBuster][load_data]")
{
    VideoBuster videoBuster;
    // VideoBuster has no outline
    auto* settings = Settings::instance();
    settings->setUsePlotForOutline(true);

    SECTION("'Normal' movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadDataSync(videoBuster,
            {{nullptr, "/dvd-bluray-verleih/183469/findet-dorie"}},
            m,
            videoBuster.scraperNativelySupports());

        CHECK(m.name() == "Findet Dorie");
        CHECK(m.certification() == Certification::FSK("0"));
        // Only year is suppported
        CHECK(m.released().toString("yyyy") == "2016");
        // Finding Dory is rated 4.6/5 (date: 2018-09-01)
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().back().rating == Approx(3.6).margin(0.5));
        CHECK(m.ratings().back().voteCount > 260);
        CHECK(m.tagline() == "Alles andere kannste vergessen.");
        CHECK(m.images().posters().size() >= 5);
        CHECK(m.images().backdrops().size() >= 4);
        CHECK(m.runtime() == 93min);

        CHECK_THAT(m.overview(), StartsWith("Mit Disney-Pixars Animationshit 'Findet Dorie' gelang"));
        CHECK_THAT(m.outline(), Equals(m.overview()));
        CHECK_THAT(m.director(), Contains("Andrew Stanton"));

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Animation");
        CHECK(genres[1] == "Kids");

        const auto tags = m.tags();
        REQUIRE(tags.size() >= 2);
        CHECK(tags[0] == "CGI-Animation");
        CHECK(tags[1] == "Pixar");

        const auto studios = m.studios();
        REQUIRE(studios.size() == 1);
        CHECK(studios[0] == "Walt Disney Studios");

        const auto countries = m.countries();
        REQUIRE(countries.size() == 1);
        CHECK(countries[0] == "USA");

        // Note: German voices
        const auto actors = m.actors();
        REQUIRE(actors.size() >= 2);
        CHECK(actors[0]->name == "Lucia Geddes");
        CHECK(actors[1]->name == "Jerome Ranft");
    }

    SECTION("Scraping movie two times does not increase actor count")
    {
        Movie m(QStringList{}); // Movie without files
        QString url = "/dvd-bluray-verleih/183469/findet-dorie";

        // load first time
        loadDataSync(videoBuster, {{nullptr, url}}, m, videoBuster.scraperNativelySupports());
        REQUIRE(m.actors().size() == 4);

        // load second time
        loadDataSync(videoBuster, {{nullptr, url}}, m, videoBuster.scraperNativelySupports());
        REQUIRE(m.actors().size() == 4);
    }
}
