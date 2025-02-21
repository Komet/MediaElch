#include "test/test_helpers.h"

#include "scrapers/movie/custom/CustomMovieScrapeJob.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "settings/Settings.h"

#include "test/helpers/scraper_helpers.h"
#include "test/unit/scrapers/custom_movie_scraper/StubMovieScraper.h"

#include <memory>
#include <vector>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

static auto makeScrapeJob(const CustomMovieScrapeJob::CustomScraperConfig& config)
{
    return std::make_unique<CustomMovieScrapeJob>(config);
}

struct CustomMovieScraperTestSetup
{
    std::vector<std::unique_ptr<test::StubMovieScraper>> scrapers;
    CustomMovieScrapeJob::CustomScraperConfig mainConfig;
};

// Setup stub scrapers; we want to test whether the custom movie scraper
// correctly dispatches details.
auto setupCustomMovieScraperStubs(std::size_t N) -> CustomMovieScraperTestSetup
{
    auto setup = CustomMovieScraperTestSetup{};
    setup.scrapers.reserve(N);
    setup.mainConfig.scraperMap.reserve(safe_int_cast<int>(N));

    for (std::size_t i = 0; i < N; ++i) {
        setup.scrapers.emplace_back(std::make_unique<test::StubMovieScraper>( //
            QStringLiteral("stub-scraper-%1").arg(i),
            nullptr));
        setup.scrapers[i]->stub_movie.setTitle(QStringLiteral("name-%1").arg(i));
        setup.scrapers[i]->stub_movie.setOverview(QStringLiteral("overview-%1").arg(i));
        setup.scrapers[i]->stub_movie.setDirector(QStringLiteral("director-%1").arg(i));
        setup.scrapers[i]->stub_movie.addGenre(QStringLiteral("first-genre-%1").arg(i));
        setup.scrapers[i]->stub_movie.addGenre(QStringLiteral("second-genre-%1").arg(i));

        MovieSet movieSet;
        movieSet.name = QStringLiteral("movie-set-name-%1").arg(i);
        movieSet.overview = QStringLiteral("movie-set-overview-%1").arg(i);
        setup.scrapers[i]->stub_movie.setSet(movieSet);

        MovieScrapeJob::Config subConfig;
        subConfig.details = mediaelch::scraper::allMovieScraperInfos();
        setup.mainConfig.scraperMap.insert(setup.scrapers[i].get(), std::move(subConfig));
    }

    return setup;
}

TEST_CASE("CustomMovieScraper scrapes correct movie details", "[CustomMovieScraper][load_data]")
{
    SECTION("uses correct details if there is no overlap")
    {
        auto setup = setupCustomMovieScraperStubs(5);

        setup.mainConfig.scraperMap[setup.scrapers[0].get()].details = {MovieScraperInfo::Overview};
        setup.mainConfig.scraperMap[setup.scrapers[1].get()].details = {MovieScraperInfo::Director};
        setup.mainConfig.scraperMap[setup.scrapers[2].get()].details = {MovieScraperInfo::Title};
        setup.mainConfig.scraperMap[setup.scrapers[3].get()].details = {MovieScraperInfo::Set};
        setup.mainConfig.scraperMap[setup.scrapers[4].get()].details = {MovieScraperInfo::Genres};

        auto scrapeJob = makeScrapeJob(setup.mainConfig);
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& movie = scrapeJob->movie();

        CHECK(movie.title() == "name-2");
        CHECK(movie.director() == "director-1");
        CHECK(movie.set().name == "movie-set-name-3");
        CHECK(movie.set().overview == "movie-set-overview-3");
        CHECK(movie.overview() == "overview-0");
        CHECK(movie.genres().size() == 2);
        CHECK_THAT(movie.genres(), Contains("first-genre-4"));
    }

    SECTION("succeeds if all scrapers have no details")
    {
        auto setup = setupCustomMovieScraperStubs(2);

        setup.mainConfig.scraperMap[setup.scrapers[0].get()].details = {};
        setup.mainConfig.scraperMap[setup.scrapers[1].get()].details = {};

        auto scrapeJob = makeScrapeJob(setup.mainConfig);
        test::scrapeMovieScraperSync(scrapeJob.get(), false);
        auto& movie = scrapeJob->movie();

        CHECK(movie.title().isEmpty());
        CHECK(movie.overview().isEmpty());
    }
}
