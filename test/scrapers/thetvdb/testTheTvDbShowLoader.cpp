#include "test/test_helpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/thetvdb/TheTvDbShowScrapeJob.h"
#include "test/scrapers/testScraperHelpers.h"
#include "test/scrapers/thetvdb/testTheTvDbHelper.h"
#include "tv_shows/TvShow.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("TheTvDb scrapes show details", "[show][TheTvDb][load_data]")
{
    waitForTheTvDbInitialized();

    SECTION("Loads minimal details for The Simpsons")
    {
        QSet<ShowScraperInfo> details{ShowScraperInfo::Title};
        ShowScrapeJob::Config config{ShowIdentifier("71663"), Locale("en-US"), details};

        auto scrapeJob = std::make_unique<TheTvDbShowScrapeJob>(getTheTvDbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.title() == "The Simpsons");
        // These fields should not be set
        CHECK(show.sortTitle().isEmpty());
        CHECK(show.actors().isEmpty());
    }

    SECTION("Loads minimal details for Scrubs in other language")
    {
        ShowScrapeJob::Config config{ShowIdentifier("76156"), Locale("pl-PL"), {ShowScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TheTvDbShowScrapeJob>(getTheTvDbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.title() == "Hoży doktorzy");
        // These fields should not be set
        CHECK(show.sortTitle().isEmpty());
        CHECK(show.actors().isEmpty());
    }

    SECTION("Loads all details for Scrubs")
    {
        TheTvDb tvdb;
        ShowScrapeJob::Config config{ShowIdentifier("76156"), Locale("en-US"), tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<TheTvDbShowScrapeJob>(getTheTvDbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE(show.tvdbId() == TvDbId("76156"));

        CHECK(show.title() == "Scrubs");
        CHECK(show.imdbId() == ImdbId("tt0285403"));
        CHECK(show.certification() == Certification("TV-PG"));
        CHECK(show.firstAired() == QDate(2001, 10, 2));
        CHECK(show.network() == "NBC");
        CHECK_THAT(show.overview(), Contains("Scrubs focuses on the lives of several people"));

        REQUIRE_FALSE(show.ratings().isEmpty());
        CHECK(show.ratings().first().rating == Approx(9).margin(0.5));
        CHECK(show.ratings().first().voteCount > 290);

        CHECK(show.runtime() == 25min);
        CHECK(show.status() == "Ended");

        // const auto& actors = show.actors();
        // REQUIRE(actors.size() > 15);
        // CHECK(actors[0]->name == "Aloma Wright");

        const auto& genres = show.genres();
        REQUIRE(!genres.empty());
        CHECK_THAT(genres[0], Contains("Comedy"));

        // CHECK(show.backdrops().size() > 25);
        // CHECK(show.posters().size() > 15);
        // TODO: CHECK(show.seasonPosters(SeasonNumber(1)).size() > 4);
        // CHECK(show.seasonBanners(SeasonNumber(1)).size() > 1);
        // CHECK(show.banners().size() > 11);
    }
    /* TheTvDb (again) has issues with other languages...
        SECTION("Loads all details for Scrubs in another Language")
        {
            TheTvDb tvdb;
            ShowScrapeJob::Config config{ShowIdentifier("76156"), Locale("de-DE"), tvdb.meta().supportedShowDetails};

            auto scrapeJob = std::make_unique<TheTvDbShowScrapeJob>(getTheTvDbApi(), config);
            scrapeTvScraperSync(scrapeJob.get());
            auto& show = scrapeJob->tvShow();

            REQUIRE(show.tvdbId() == TvDbId("76156"));

            CHECK(show.title() == "Scrubs");
            CHECK(show.certification() == Certification("TV-PG"));
            CHECK(show.firstAired() == QDate(2001, 10, 2));
            CHECK(show.network() == "NBC");
            CHECK_THAT(show.overview(),
                StartsWith("Mittelpunkt und Ich-Erzähler der Serie ist der junge Mediziner John Michael Dorian"));

            REQUIRE_FALSE(show.ratings().isEmpty());
            CHECK(show.ratings().first().rating == Approx(9).margin(0.5));
            CHECK(show.ratings().first().voteCount > 290);

            CHECK(show.runtime() == 25min);
            CHECK(show.status() == "Ended");

            // const auto& actors = show.actors();
            // REQUIRE(actors.size() > 15);
            // CHECK(actors[0]->name == "Aloma Wright");

            const auto& genres = show.genres();
            REQUIRE(!genres.empty());
            CHECK_THAT(genres[0], Contains("Comedy"));

            // CHECK(show.backdrops().size() > 25);
            // CHECK(show.posters().size() > 15);
            // TODO: CHECK(show.seasonPosters(SeasonNumber(1)).size() > 4);
            // CHECK(show.seasonBanners(SeasonNumber(1)).size() > 1);
            // CHECK(show.banners().size() > 11);
        }
    */
}
