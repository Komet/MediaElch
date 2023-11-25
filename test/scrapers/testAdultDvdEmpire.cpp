#include "test/test_helpers.h"

#include "scrapers/movie/adultdvdempire/AdultDvdEmpire.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpireScrapeJob.h"
#include "scrapers/movie/adultdvdempire/AdultDvdEmpireSearchJob.h"
#include "test/helpers/scraper_helpers.h"

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

TEST_CASE("AdultDvdEmpire returns valid search results", "[movie][AdultDvdEmpire][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Magic Mike", mediaelch::Locale::English};
        auto* searchJob = new AdultDvdEmpireSearchJob(getAdultDvdEmpireApi(), config);
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 2);
        CHECK_THAT(scraperResults[0].title, Matches("\\[(DVD|VOD)\\] Magic Mike XXXL"));
    }
}

TEST_CASE("AdultDvdEmpire scrapes correct movie details", "[movie][AdultDvdEmpire][load_data]")
{
    AdultDvdEmpire hm;

    SECTION("Movie has correct details for DVD movie")
    {
        auto scrapeJob = makeScrapeJob("/1745335/magic-mike-xxxl-porn-movies.html");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        test::scraper::compareAgainstReference(m, "scrapers/ade/DVD-Magic-Mike-1745335");
    }

    SECTION("Movie has correct details for VOD movie")
    {
        auto scrapeJob = makeScrapeJob("/1670507/50-shades-of-pink-porn-videos.html");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE_THAT(m.name(), StartsWith("50 Shades Of Pink"));
        test::scraper::compareAgainstReference(m, "scrapers/ade/VOD-50-Shades-1670507");
    }
}
