#include "test/test_helpers.h"

#include "scrapers/movie/videobuster/VideoBuster.h"
#include "scrapers/movie/videobuster/VideoBusterSearchJob.h"
#include "settings/Settings.h"
#include "test/scrapers/testScraperHelpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static VideoBusterApi& getVideoBusterApi()
{
    static auto api = std::make_unique<VideoBusterApi>();
    return *api;
}

// VideoBuster is a German website so search results and movie
// details in these tests are German as well.

TEST_CASE("VideoBuster returns valid search results", "[VideoBuster][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Findet Dorie", mediaelch::Locale("de-DE")};
        auto* searchJob = new VideoBusterSearchJob(getVideoBusterApi(), config);
        const auto scraperResults = searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Findet Dorie");
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
            {{nullptr, MovieIdentifier("/dvd-bluray-verleih/183469/findet-dorie")}},
            m,
            videoBuster.scraperNativelySupports());

        // Note: VideoBuster is a German site, i.e. will contain German voices
        REQUIRE(m.name() == "Findet Dorie");
        test::scraper::compareAgainstReference(m, "scrapers/video-buster/Findet-Dorie-183469");
    }

    SECTION("Scraping movie two times does not increase actor count")
    {
        Movie m(QStringList{}); // Movie without files
        MovieIdentifier url("/dvd-bluray-verleih/183469/findet-dorie");

        // load first time
        loadDataSync(videoBuster, {{nullptr, MovieIdentifier(url)}}, m, videoBuster.scraperNativelySupports());
        REQUIRE(m.actors().size() == 4);

        // load second time
        loadDataSync(videoBuster, {{nullptr, MovieIdentifier(url)}}, m, videoBuster.scraperNativelySupports());
        REQUIRE(m.actors().size() == 4);
    }
}
