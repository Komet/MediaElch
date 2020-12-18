#include "test/test_helpers.h"

#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "scrapers/tv_show/tvmaze/TvMazeShowScrapeJob.h"
#include "test/scrapers/testScraperHelpers.h"
#include "test/scrapers/tvmaze/testTvMazeHelper.h"
#include "tv_shows/TvShow.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("TvMaze scrapes show details", "[show][TvMaze][load_data]")
{
    SECTION("Loads minimal details for The Simpsons")
    {
        QSet<ShowScraperInfo> details{ShowScraperInfo::Title};
        ShowScrapeJob::Config config{ShowIdentifier("83"), Locale::English, details};

        auto scrapeJob = std::make_unique<TvMazeShowScrapeJob>(getTvMazeApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.title() == "The Simpsons");
        CHECK(show.firstAired() == QDate(1989, 12, 17));
        CHECK(show.ratings().size() == 1);
        CHECK(show.sortTitle().isEmpty());
    }

    SECTION("Loads all details for The Simpsons")
    {
        TvMaze tvdb;
        ShowScrapeJob::Config config{ShowIdentifier("83"), Locale::English, tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TvMazeShowScrapeJob>(getTvMazeApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.imdbId() == ImdbId("tt0096697"));
        CHECK(show.tvdbId() == TvDbId("71663"));
        CHECK(show.title() == "The Simpsons");

        // TVmaze does not support the certification
        CHECK(show.firstAired() == QDate(1989, 12, 17));
        CHECK(show.status() == "Running");
        CHECK(show.network() == "FOX");
        CHECK_THAT(show.overview(), StartsWith("The Simpsons is the longest running scripted"));

        REQUIRE_FALSE(show.ratings().isEmpty());
        CHECK(show.ratings().first().rating == Approx(8.7).margin(0.5));
        // TvMaze has no vote count

        CHECK(show.runtime() == 30min);
        CHECK(show.posters().size() == 30);
        CHECK(show.backdrops().size() == 1);
        CHECK(show.seasonPosters(SeasonNumber::NoSeason, true).size() > 30);

        const auto& genres = show.genres();
        REQUIRE(!genres.empty());
        CHECK_THAT(genres[0], Contains("Comedy"));

        const auto& actors = show.actors();
        REQUIRE(actors.size() > 5);
        CHECK(actors[0]->name == "Dan Castellaneta");
        CHECK(actors[0]->role == "Homer Simpson");
        CHECK(actors[0]->id == "14854");
        CHECK(actors[0]->thumb == "http://static.tvmaze.com/uploads/images/original_untouched/0/963.jpg");
    }
}
