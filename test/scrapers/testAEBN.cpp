#include "test/test_helpers.h"

#include "scrapers/movie/aebn/AEBN.h"
#include "scrapers/movie/aebn/AebnSearchJob.h"
#include "test/mocks/settings/MockScraperSettings.h"
#include "test/scrapers/testScraperHelpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static AebnApi& getAebnApi()
{
    static auto api = std::make_unique<AebnApi>();
    return *api;
}

/// \brief Loads movie data synchronously
static void loadAebnMoviesSync(AEBN& scraper, QHash<MovieScraper*, MovieIdentifier> ids, Movie& movie)
{
    const auto infos = scraper.meta().supportedDetails;
    loadDataSync(scraper, ids, movie, infos);
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
        CHECK(scraperResults[0].identifier.str() == "188623");
    }
}

TEST_CASE("AEBN scrapes correct movie details", "[AEBN][load_data]")
{
    AEBN aebn;
    MockScraperSettings settings(aebn.meta().identifier);
    settings.key_string_map["Genre"] = "101"; // straight
    aebn.loadSettings(settings);


    SECTION("Movie has correct details")
    {
        Movie m(QStringList{}); // Movie without files
        loadAebnMoviesSync(aebn, {{nullptr, MovieIdentifier("188623")}}, m);

        REQUIRE_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        test::compareAgainstReference(m, "scrapers/aebn/Magic-Mike-188623");
    }

    SECTION("Movie has correct set")
    {
        Movie m(QStringList{}); // Movie without files
        loadAebnMoviesSync(aebn, {{nullptr, MovieIdentifier("159236")}}, m);

        REQUIRE(m.name() == "M Is For Mischief 3");
        REQUIRE(m.set().name == "M Is For Mischief");
        test::compareAgainstReference(m, "scrapers/aebn/M-Is-For-Mischief-159236");
    }
}
