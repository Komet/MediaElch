#include "test/test_helpers.h"

#include "scrapers/movie/videobuster/VideoBuster.h"
#include "scrapers/movie/videobuster/VideoBusterScrapeJob.h"
#include "scrapers/movie/videobuster/VideoBusterSearchJob.h"
#include "settings/Settings.h"
#include "test/helpers/scraper_helpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static VideoBusterApi& getVideoBusterApi()
{
    static auto api = std::make_unique<VideoBusterApi>();
    return *api;
}

static MovieScrapeJob::Config makeVideoBusterConfig(const QString& id)
{
    static auto videobuster = std::make_unique<VideoBuster>();
    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(id);
    config.details = videobuster->meta().supportedDetails;
    config.locale = videobuster->meta().defaultLocale;
    return config;
}

static auto makeScrapeJob(const QString& id)
{
    return std::make_unique<VideoBusterScrapeJob>(getVideoBusterApi(), makeVideoBusterConfig(id));
}

// VideoBuster is a German website so search results and movie
// details in these tests are German as well.

TEST_CASE("VideoBuster returns valid search results", "[movie][VideoBuster][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Findet Dorie", mediaelch::Locale("de-DE")};
        auto* searchJob = new VideoBusterSearchJob(getVideoBusterApi(), config);
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Findet Dorie");
    }
}


TEST_CASE("VideoBuster scrapes correct movie details", "[movie][VideoBuster][load_data]")
{
    VideoBuster videoBuster;
    // VideoBuster has no outline
    auto* settings = Settings::instance();
    settings->setUsePlotForOutline(true);

    SECTION("'Normal' movie has correct details")
    {
        auto scrapeJob = makeScrapeJob("/dvd-bluray-verleih/183469/findet-dorie");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        // Note: VideoBuster is a German site, i.e. will contain German voices
        REQUIRE(m.title() == "Findet Dorie");
        test::scraper::compareAgainstReference(m, "scrapers/video-buster/Findet-Dorie-183469");
    }
}
