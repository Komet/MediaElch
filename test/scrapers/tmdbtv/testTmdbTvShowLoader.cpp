#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"
#include "test/scrapers/testScraperHelpers.h"
#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("TmdbTv scrapes show details", "[show][TmdbTv][load_data]")
{
    waitForTmdbTvInitialized();

    SECTION("Loads minimal details for The Simpsons")
    {
        QSet<ShowScraperInfo> details{ShowScraperInfo::Title};
        ShowScrapeJob::Config config{ShowIdentifier("456"), Locale("en-US"), details};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.title() == "The Simpsons");
        CHECK(show.firstAired() == QDate(1989, 12, 17));
        CHECK(show.ratings().size() == 1);
        CHECK(show.sortTitle().isEmpty());
        CHECK(show.actors().hasActors());
        CHECK_FALSE(show.ratings().isEmpty());
    }

    SECTION("Loads minimal details for Scrubs in other language")
    {
        ShowScrapeJob::Config config{ShowIdentifier("4556"), Locale("de-DE"), {ShowScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.title() == "Scrubs - Die Anfänger");
        CHECK(show.originalTitle() == "Scrubs");
        CHECK(show.firstAired() == QDate(2001, 10, 2));
        CHECK(show.sortTitle().isEmpty());
        CHECK(show.actors().hasActors());
        CHECK_FALSE(show.ratings().isEmpty());
    }

    SECTION("Loads all details for Scrubs")
    {
        TmdbTv tvdb;
        ShowScrapeJob::Config config{ShowIdentifier("4556"), Locale("en-US"), tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.imdbId() == ImdbId("tt0285403"));
        CHECK(show.tvdbId() == TvDbId("76156"));
        CHECK(show.title() == "Scrubs");
        CHECK(show.certification() == Certification("TV-14"));
        CHECK(show.firstAired() == QDate(2001, 10, 2));
        CHECK(show.status() == "Ended");
        CHECK(show.network() == "ABC, NBC");
        CHECK_THAT(show.overview(), StartsWith("In the unreal world of Sacred Heart Hospital"));

        REQUIRE_FALSE(show.ratings().isEmpty());
        CHECK(show.ratings().first().rating == Approx(7.9).margin(0.5));
        CHECK(show.ratings().first().voteCount > 750);

        CHECK(show.runtime() == 24min);
        CHECK(show.posters().size() == 1);
        CHECK(show.backdrops().size() == 1);
        CHECK(show.seasonPosters(SeasonNumber::NoSeason, true).size() > 9);

        const auto& genres = show.genres();
        REQUIRE(!genres.empty());
        CHECK_THAT(genres[0], Contains("Comedy"));

        const auto& actors = show.actors().actors();
        REQUIRE(actors.size() > 5);
        CHECK(actors[0]->name == "Zach Braff");
        CHECK(actors[0]->role == R"(John "J.D." Dorian)");
        CHECK(actors[0]->id == "5367");
        CHECK(actors[0]->thumb == "https://image.tmdb.org/t/p/original/l7Z2HWaU1DtwvSMXNKIoINz1lo7.jpg");

        const auto& tags = show.tags();
        CHECK(tags.size() > 5);
        CHECK_THAT(tags, Contains("friendship"));
        CHECK_THAT(tags, Contains("hospital"));
        CHECK_THAT(tags, Contains("doctor"));
        CHECK_THAT(tags, Contains("sitcom"));
    }

    SECTION("Loads all details for Scrubs in another Language")
    {
        TmdbTv tvdb;
        ShowScrapeJob::Config config{ShowIdentifier("4556"), Locale("de-DE"), tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.imdbId() == ImdbId("tt0285403"));
        CHECK(show.tvdbId() == TvDbId("76156"));

        CHECK(show.title() == "Scrubs - Die Anfänger");
        CHECK(show.originalTitle() == "Scrubs");
        CHECK(show.certification() == Certification("16")); // German format
        CHECK(show.firstAired() == QDate(2001, 10, 2));
        CHECK(show.status() == "Ended");
        CHECK(show.network() == "ABC, NBC");
        CHECK_THAT(show.overview(), StartsWith("Der junge Mediziner John “J.D.” Dorian"));

        REQUIRE_FALSE(show.ratings().isEmpty());
        CHECK(show.ratings().first().rating == Approx(7.9).margin(0.5));
        CHECK(show.ratings().first().voteCount > 770);

        CHECK(show.runtime() == 24min);
        CHECK(show.posters().size() == 1);
        CHECK(show.backdrops().size() == 1);
        CHECK(show.seasonPosters(SeasonNumber::NoSeason, true).size() > 9);

        const auto& genres = show.genres();
        REQUIRE(!genres.empty());
        CHECK_THAT(genres[0], Contains("Komödie"));

        const auto& actors = show.actors().actors();
        REQUIRE(actors.size() > 5);
        int zachIndex = -1;
        for (int i = 0; i < actors.size(); ++i) {
            const Actor* actor = actors[i];
            if (actor->name == "Zach Braff") {
                zachIndex = i;
            }
        }
        REQUIRE(zachIndex >= 0);
        CHECK(actors[zachIndex]->name == "Zach Braff");
        CHECK(actors[zachIndex]->role == R"(John "J.D." Dorian)");
        CHECK(actors[zachIndex]->id == "5367");
        CHECK(actors[zachIndex]->thumb == "https://image.tmdb.org/t/p/original/l7Z2HWaU1DtwvSMXNKIoINz1lo7.jpg");

        const auto& tags = show.tags();
        CHECK(tags.size() > 5);
        CHECK_THAT(tags, Contains("friendship"));
        CHECK_THAT(tags, Contains("hospital"));
        CHECK_THAT(tags, Contains("doctor"));
        CHECK_THAT(tags, Contains("sitcom"));
    }


    SECTION("Loads the full cast for Stargate SG-1")
    {
        // Test everything, including cast.
        TmdbTv tvdb;
        ShowScrapeJob::Config config{ShowIdentifier("4629"), Locale("en-US"), tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TmdbTvShowScrapeJob>(getTmdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.imdbId() == ImdbId("tt0118480"));
        CHECK(show.tvdbId() == TvDbId("72449"));
        CHECK(show.title() == "Stargate SG-1");
        CHECK(show.certification() == Certification("TV-PG"));
        CHECK(show.firstAired() == QDate(1997, 7, 27));
        CHECK(show.status() == "Ended");
        CHECK(show.network() == "Showtime, Syfy");
        CHECK_THAT(show.overview(), StartsWith("The story of Stargate SG-1 begins"));

        REQUIRE_FALSE(show.ratings().isEmpty());
        CHECK(show.ratings().first().rating == Approx(8.3).margin(0.5));
        CHECK(show.ratings().first().voteCount > 1150);

        CHECK(show.runtime() == 42min);
        CHECK(show.posters().size() == 1);
        CHECK(show.backdrops().size() == 1);
        CHECK(show.seasonPosters(SeasonNumber::NoSeason, true).size() > 10);

        const auto& genres = show.genres();
        REQUIRE(!genres.empty());
        CHECK_THAT(genres[0], Contains("Sci-Fi & Fantasy"));

        // We used TMDb's `credits` field in the past. This field only contains
        // the actors of the _last_ season.  Newer MediaElch versions use
        // `aggregate_credits`, which also includes e.g. "Richard Dean Anderson".
        // There are only 6 actors listed for the last season.
        const auto& actors = show.actors().actors();
        // Yes, there are really 680 actors.
        REQUIRE(actors.size() > 680);
        CHECK(actors[0]->name == "Richard Dean Anderson");
        CHECK(actors[0]->role == "Jack O'Neill");
        CHECK(actors[0]->id == "26085");
        CHECK(actors[0]->thumb == "https://image.tmdb.org/t/p/original/w9Wi0OUEFGy9vMUpiZjj9GLzpag.jpg");

        CHECK(actors[1]->name == "Amanda Tapping");
        CHECK(actors[1]->role == "Samantha Carter");
        CHECK(actors[1]->id == "26087");
        CHECK(actors[1]->thumb == "https://image.tmdb.org/t/p/original/8ZiETPxUFtgrtWL0LgMwPP8ytuK.jpg");

        CHECK(actors[6]->name == "Ben Browder");
        CHECK(actors[6]->role == "Cameron Mitchell");
        CHECK(actors[6]->id == "26048");
        CHECK(actors[6]->thumb == "https://image.tmdb.org/t/p/original/28gdcjphnh7zjpWxgWWVvv9XMA7.jpg");

        const auto& tags = show.tags();
        CHECK(tags.size() > 5);
        CHECK_THAT(tags, Contains("space travel"));
        CHECK_THAT(tags, Contains("alien"));
        CHECK_THAT(tags, Contains("space"));
        CHECK_THAT(tags, Contains("military"));
    }
}
