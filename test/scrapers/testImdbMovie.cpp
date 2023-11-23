#include "test/test_helpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/imdb/ImdbMovieScrapeJob.h"
#include "scrapers/movie/imdb/ImdbMovieSearchJob.h"
#include "test/helpers/scraper_helpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static ImdbApi& getImdbApi()
{
    static auto api = std::make_unique<ImdbApi>();
    return *api;
}

static MovieScrapeJob::Config makeImdbConfig(const QString& id)
{
    static auto imdb = std::make_unique<ImdbMovie>();
    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(id);
    config.details = imdb->meta().supportedDetails;
    config.locale = imdb->meta().defaultLocale;
    return config;
}

static auto makeScrapeJob(const QString& id, bool loadAllTags = false)
{
    return std::make_unique<ImdbMovieScrapeJob>(getImdbApi(), makeImdbConfig(id), loadAllTags);
}

TEST_CASE("IMDb returns valid search results", "[IMDb][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Finding Dory", mediaelch::Locale::English};
        auto* searchJob = new ImdbMovieSearchJob(getImdbApi(), config);
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 2);
        CHECK(scraperResults[0].title == "Finding Dory");
        CHECK(scraperResults[0].released == QDate(2016, 1, 1));
        // Second result changes frequently but contains "Finding"
        CHECK(scraperResults[1].title.contains("Finding"));
    }

    SECTION("Search by IMDb ID returns correct results")
    {
        MovieSearchJob::Config config{"tt2277860", mediaelch::Locale::English};
        auto* searchJob = new ImdbMovieSearchJob(getImdbApi(), config);
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        // "Search" by ID actually loads the movie page, therefore only one result
        REQUIRE(scraperResults.length() == 1);
        CHECK_THAT(scraperResults[0].title, Matches("Finding Dory|Findet Dorie")); // Maintainer is German
        CHECK(scraperResults[0].released.toString("yyyy") == "2016");
    }
}

TEST_CASE("IMDb scrapes correct movie details", "[scraper][IMDb][load_data]")
{
    SECTION("'Normal' movie has correct details")
    {
        auto scrapeJob = makeScrapeJob("tt2277860");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE(m.imdbId() == ImdbId("tt2277860"));
        CHECK(m.tmdbId() == TmdbId::NoId);
        CHECK_THAT(m.name(), Matches("Finding Dory|Findet Dorie")); // Maintainer is German
        CHECK(m.originalName() == "Finding Dory");
        CHECK(m.certification() == Certification("PG"));
        CHECK(m.released().toString("yyyy-MM-dd") == "2016-09-29");
        // Finding Dory is rated 7.3 (date: 2018-08-31)
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().first().rating == Approx(7).margin(0.5));
        CHECK(m.ratings().first().voteCount > 6300);
        // Movie is not in top 250
        CHECK(m.top250() == 0);
        // Tagline may be different on each run, so we only
        // check if it is existent.
        CHECK_FALSE(m.tagline().isEmpty());
        CHECK(m.images().posters().size() == 1);
        CHECK(m.runtime() == 97min);

        CHECK_THAT(m.overview(), StartsWith("Dory is a wide-eyed, blue tang fish"));
        CHECK_THAT(m.outline(), StartsWith("Friendly but forgetful blue tang Dory"));
        CHECK_THAT(m.director(), Contains("Andrew Stanton"));
        CHECK_THAT(m.director(), Contains("Angus MacLane"));
        CHECK_THAT(m.writer(), Contains("Andrew Stanton"));
        CHECK_THAT(m.writer(), Contains("Victoria Strouse"));

        REQUIRE(m.genres().size() >= 2);
        REQUIRE(m.tags().size() >= 2);
        REQUIRE(m.studios().size() >= 2);
        REQUIRE(m.countries().size() >= 1);
        REQUIRE(m.actors().actors().size() >= 2);

        test::scraper::compareAgainstReference(m, "scrapers/imdb/Finding_Dory_tt2277860");
    }

    SECTION("'Top 250' movie has correct details")
    {
        auto scrapeJob = makeScrapeJob("tt0111161");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE(m.imdbId() == ImdbId("tt0111161"));
        test::scraper::compareAgainstReference(m, "scrapers/imdb/The_Shawshank_Redemption_tt0111161");
    }

    SECTION("Loads tags correctly")
    {
        SECTION("'load all tags' is true")
        {
            auto scrapeJob = makeScrapeJob("tt0111161", true);
            test::scrapeMovieScraperSync(scrapeJob.get(), false);
            auto& m = scrapeJob->movie();

            const auto tags = m.tags();
            REQUIRE(tags.size() >= 20);
            CHECK_THAT(tags, Contains("prison"));
            CHECK_THAT(tags, Contains("suicide"));
        }

        SECTION("'load all tags' is false")
        {
            auto scrapeJob = makeScrapeJob("tt0111161", false);
            test::scrapeMovieScraperSync(scrapeJob.get(), false);
            auto& m = scrapeJob->movie();

            const auto tags = m.tags();
            REQUIRE(tags.size() >= 2);
            REQUIRE(tags.size() <= 20);
            CHECK(tags[0] == "reading-lesson");
        }
    }

    SECTION("IMDb loads original title")
    {
        auto scrapeJob = makeScrapeJob("tt2987732");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE(m.imdbId() == ImdbId("tt2987732"));
        // translated english version / german original
        CHECK_THAT(m.name(), Matches("Suck Me Shakespeer|Fack ju Göhte"));
        if (m.name() == "Suck Me Shakespeer") {
            // original german title
            // Only appears if the site is the English version. If it's the German one,
            // no original title is shown.
            CHECK(m.originalName() == "Fack ju Göhte");
        }
    }

    SECTION("Movie with multiple countries is loaded")
    {
        auto scrapeJob = makeScrapeJob("tt1663662");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE(m.imdbId() == ImdbId("tt1663662"));
        test::scraper::compareAgainstReference(m, "scrapers/imdb/Pacific_Rim_tt1663662");
    }

    SECTION("Lesser known indian movie has correct details")
    {
        auto scrapeJob = makeScrapeJob("tt3159708");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE(m.imdbId() == ImdbId("tt3159708"));
        test::scraper::compareAgainstReference(m, "scrapers/imdb/Welcome_Back_tt3159708");
    }

    SECTION("Godfather's Rating is loaded")
    {
        // The 2020-12 remake of IMDb's site has different rating layouts.
        // Godfather is one example.
        auto scrapeJob = makeScrapeJob("tt0068646");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        CHECK(m.imdbId() == ImdbId("tt0068646"));
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().first().rating == Approx(9.2).margin(0.5));

        test::scraper::compareAgainstReference(m, "scrapers/imdb/Godfather_tt0068646");
    }
}
