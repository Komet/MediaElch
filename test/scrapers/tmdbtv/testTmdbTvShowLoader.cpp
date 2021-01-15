#include "test/test_helpers.h"

#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"
#include "test/scrapers/testScraperHelpers.h"
#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"
#include "tv_shows/TvShow.h"

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
        CHECK(show.firstAired() == QDate(1989, 12, 16));
        CHECK(show.ratings().size() == 1);
        CHECK(show.sortTitle().isEmpty());
        CHECK_FALSE(show.actors().isEmpty());
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
        CHECK_FALSE(show.actors().isEmpty());
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

        const auto& actors = show.actors();
        REQUIRE(actors.size() > 5);
        CHECK(actors[0]->name == "Zach Braff");
        CHECK(actors[0]->role == R"(John "J.D." Dorian)");
        CHECK(actors[0]->id == "5367");
        CHECK(actors[0]->thumb == "https://image.tmdb.org/t/p/original/wUAj6juL6HErqEJ1GtuI63rbVea.jpg");

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

        const auto& actors = show.actors();
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
        CHECK(actors[zachIndex]->thumb == "https://image.tmdb.org/t/p/original/wUAj6juL6HErqEJ1GtuI63rbVea.jpg");

        const auto& tags = show.tags();
        CHECK(tags.size() > 5);
        CHECK_THAT(tags, Contains("friendship"));
        CHECK_THAT(tags, Contains("hospital"));
        CHECK_THAT(tags, Contains("doctor"));
        CHECK_THAT(tags, Contains("sitcom"));
    }
}
