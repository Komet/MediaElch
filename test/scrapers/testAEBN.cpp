#include "test/test_helpers.h"

#include "scrapers/movie/aebn/AEBN.h"
#include "scrapers/movie/aebn/AebnScrapeJob.h"
#include "scrapers/movie/aebn/AebnSearchJob.h"
#include "test/helpers/scraper_helpers.h"

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

static auto makeScrapeJob(QString id, QString genre)
{
    // 101 -> straight
    // 102 -> gay
    return std::make_unique<AebnScrapeJob>(getAebnApi(), makeAebnConfig(std::move(id)), std::move(genre));
}

TEST_CASE("AEBN returns valid search results", "[AEBN][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Magic Mike XXXL", mediaelch::Locale::English, true};
        auto* searchJob = new AebnSearchJob(getAebnApi(), config, "101");
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Magic Mike XXXL: A Hardcore Parody");
    }
}

TEST_CASE("AEBN scrapes correct movie details", "[AEBN][load_data]")
{
    SECTION("Movie has correct details")
    {
        auto scrapeJob = makeScrapeJob("188623", "101");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        test::scraper::compareAgainstReference(m, "scrapers/aebn/Magic-Mike-188623");
    }

    SECTION("Movie has correct set")
    {
        auto scrapeJob = makeScrapeJob("159236", "101");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE(m.name() == "M Is For Mischief 3");
        REQUIRE(m.set().name == "M Is For Mischief");
        test::scraper::compareAgainstReference(m, "scrapers/aebn/M-Is-For-Mischief-159236");
    }
}
