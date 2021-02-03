#include "test/test_helpers.h"

#include "scrapers/movie/adultdvdempire/AdultDvdEmpire.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpireScrapeJob.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpireSearchJob.h"
#include "test/scrapers/testScraperHelpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static AdultDvdEmpireApi& getAdultDvdEmpireApi()
{
    static auto api = std::make_unique<AdultDvdEmpireApi>();
    return *api;
}

static MovieScrapeJob::Config makeAdultDvdEmpireConfig(QString id)
{
    static auto ade = std::make_unique<AdultDvdEmpire>();
    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(id);
    config.details = ade->meta().supportedDetails;
    config.locale = ade->meta().defaultLocale;
    return config;
}

static auto makeScrapeJob(QString id)
{
    return std::make_unique<AdultDvdEmpireScrapeJob>(getAdultDvdEmpireApi(), makeAdultDvdEmpireConfig(id));
}

TEST_CASE("AdultDvdEmpire returns valid search results", "[AdultDvdEmpire][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Magic Mike", mediaelch::Locale::English};
        auto* searchJob = new AdultDvdEmpireSearchJob(getAdultDvdEmpireApi(), config);
        const auto scraperResults = searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].title == "[DVD] Magic Mike XXXL");
    }
}

TEST_CASE("AdultDvdEmpire scrapes correct movie details", "[AdultDvdEmpire][load_data]")
{
    AdultDvdEmpire hm;

    SECTION("Movie has correct details")
    {
        auto scrapeJob = makeScrapeJob("/1745335/magic-mike-xxxl-porn-movies.html");
        scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        CHECK_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        CHECK(m.imdbId() == ImdbId::NoId);
        CHECK(m.tmdbId() == TmdbId::NoId);
        CHECK(m.released().toString("yyyy") == "2015");

        CHECK(m.images().posters().size() == 1);
        CHECK(m.images().backdrops().size() > 70);
        CHECK(m.runtime() == 201min);

        CHECK_THAT(m.overview(), Contains("Award-Winning Director Brad Armstrong brings you"));
        CHECK(m.director() == "Brad Armstrong");

        const auto genres = m.genres();
        REQUIRE(genres.size() == 10);
        CHECK(genres[0] == "Big Budget");
        CHECK_THAT(genres[1], StartsWith("Big"));

        const auto studios = m.studios();
        REQUIRE(!studios.empty());
        CHECK(studios[0] == "Wicked Pictures");

        const auto actors = m.actors();
        REQUIRE(actors.size() > 15);
        bool foundActor = false;
        QString actorThumb;
        for (const auto* actor : actors) {
            if (actor->name == "Adriana Chechik") {
                foundActor = true;
                actorThumb = actor->thumb;
                break;
            }
        }
        CHECK(foundActor);
        CHECK(actorThumb == "https://imgs1cdn.adultempire.com/actors/652646h.jpg");
    }
}
