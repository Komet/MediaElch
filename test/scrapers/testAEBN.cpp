#include "test/test_helpers.h"

#include "scrapers/movie/aebn/AEBN.h"
#include "scrapers/movie/aebn/AebnScrapeJob.h"
#include "scrapers/movie/aebn/AebnSearchJob.h"
#include "test/scrapers/testScraperHelpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static AebnApi& getAebnApi()
{
    static auto api = std::make_unique<AebnApi>();
    return *api;
}

static MovieScrapeJob::Config makeAebnConfig(QString id)
{
    static auto aebn = std::make_unique<AEBN>();
    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(id);
    config.details = aebn->meta().supportedDetails;
    config.locale = aebn->meta().defaultLocale;
    return config;
}

static auto makeScrapeJob(QString id)
{
    return std::make_unique<AebnScrapeJob>(getAebnApi(), makeAebnConfig(id));
}

TEST_CASE("AEBN returns valid search results", "[AEBN][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Magic Mike XXXL", mediaelch::Locale::English, true};
        auto* searchJob = new AebnSearchJob(getAebnApi(), config, "straight");
        const auto scraperResults = searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Magic Mike XXXL: A Hardcore Parody");
    }
}

TEST_CASE("AEBN scrapes correct movie details", "[AEBN][load_data]")
{
    SECTION("Movie has correct details")
    {
        auto scrapeJob = makeScrapeJob("188623");
        scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        CHECK(m.imdbId() == ImdbId::NoId);
        CHECK(m.tmdbId() == TmdbId::NoId);
        CHECK(m.released().toString("yyyy") == "2015");
        CHECK(m.images().posters().size() == 1);
        CHECK(m.runtime() == 200min);

        CHECK_THAT(m.overview(), Contains("Magic Mike, Dallas, and the rest "));
        CHECK(m.director() == "Brad Armstrong");

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Feature");
        CHECK(genres[1] == "Adult Humor");

        const auto studios = m.studios();
        REQUIRE(!studios.empty());
        CHECK(studios[0] == "Wicked Pictures");

        const auto actors = m.actors();
        REQUIRE(actors.size() == 19);
        CHECK(actors[0]->name == "Misty Stone");
        CHECK(actors[1]->name == "Asa Akira");
    }

    SECTION("Movie has correct set")
    {
        auto scrapeJob = makeScrapeJob("159236");
        scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        CHECK(m.name() == "M Is For Mischief 3");
        CHECK(m.set().name == "M Is For Mischief");
    }
}
