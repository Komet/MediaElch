#include "test/test_helpers.h"

#include "scrapers/movie/videobuster/VideoBuster.h"
#include "scrapers/movie/videobuster/VideoBusterScrapeJob.h"
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

static MovieScrapeJob::Config makeVideoBusterConfig(QString id)
{
    static auto videobuster = std::make_unique<VideoBuster>();
    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(id);
    config.details = videobuster->meta().supportedDetails;
    config.locale = videobuster->meta().defaultLocale;
    return config;
}

static auto makeScrapeJob(QString id)
{
    return std::make_unique<VideoBusterScrapeJob>(getVideoBusterApi(), makeVideoBusterConfig(id));
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
        auto scrapeJob = makeScrapeJob("/dvd-bluray-verleih/183469/findet-dorie");
        scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

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
}
