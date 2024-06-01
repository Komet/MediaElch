#include "test/test_helpers.h"

#include "scrapers/movie/hotmovies/HotMovies.h"
#include "scrapers/movie/hotmovies/HotMoviesScrapeJob.h"
#include "scrapers/movie/hotmovies/HotMoviesSearchJob.h"
#include "test/helpers/scraper_helpers.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static HotMoviesApi& getHotMoviesApi()
{
    static auto api = std::make_unique<HotMoviesApi>();
    return *api;
}

static MovieScrapeJob::Config makeHotMoviesConfig(QString id)
{
    static auto hotMovies = std::make_unique<HotMovies>();
    MovieScrapeJob::Config config;
    config.identifier = MovieIdentifier(id);
    config.details = hotMovies->meta().supportedDetails;
    config.locale = hotMovies->meta().defaultLocale;
    return config;
}

static auto makeScrapeJob(QString id)
{
    return std::make_unique<HotMoviesScrapeJob>(getHotMoviesApi(), makeHotMoviesConfig(id));
}

static void cleanupMovie(Movie& movie)
{
    QString overview = movie.overview();
    overview.truncate(100);
    overview.replace("fuck", "…", Qt::CaseSensitivity::CaseInsensitive);
    overview.replace("porn", "…", Qt::CaseSensitivity::CaseInsensitive);
    overview += "…";
    movie.setOverview(overview);
}

TEST_CASE("HotMovies returns valid search results", "[movie][HotMovies][search]")
{
    SECTION("Search by movie name returns correct results")
    {
        MovieSearchJob::Config config{"Magic Mike XXXL", mediaelch::Locale::English};
        auto* searchJob = new HotMoviesSearchJob(getHotMoviesApi(), config);
        const auto scraperResults = test::searchMovieScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK_THAT(scraperResults[0].title, Contains("Magic Mike XXXL"));
        // The identifier should be a full URL.
        CHECK_THAT(scraperResults[0].identifier.str(), StartsWith("https://"));
    }
}

TEST_CASE("HotMovies scrapes correct movie details", "[movie][HotMovies][load_data]")
{
    HotMovies hm;

    SECTION("Movie has correct details")
    {
        auto scrapeJob = makeScrapeJob("https://www.hotmovies.com/1747611/magic-mike-xxxl-porn-video.html");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        cleanupMovie(m);
        test::scraper::compareAgainstReference(m, "scrapers/hot-movies/Magic-Mike-292788");
    }

    SECTION("Movie has correct set")
    {
        auto scrapeJob = makeScrapeJob("https://www.hotmovies.com/1616127/m-is-for-mischief-no-3-porn-video.html");
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& m = scrapeJob->movie();

        REQUIRE_THAT(m.name(), StartsWith("\"M\" Is For Mischief No. 3"));
        REQUIRE(m.set().name == "\"M\" Is For Mischief");
        cleanupMovie(m);
        test::scraper::compareAgainstReference(m, "scrapers/hot-movies/M-Is-For-Mischief-214343");
    }
}
