#include "test/test_helpers.h"

#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h"
#include "test/scrapers/imdbtv/testImdbTvHelper.h"
#include "test/scrapers/testScraperHelpers.h"
#include "tv_shows/TvShow.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("ImdbTv scrapes show details", "[show][ImdbTv][load_data]")
{
    SECTION("Loads minimal details for The Simpsons")
    {
        QSet<ShowScraperInfo> details{ShowScraperInfo::Title};
        ShowScrapeJob::Config config{ShowIdentifier("tt0096697"), Locale("en-US"), details};

        auto scrapeJob = std::make_unique<ImdbTvShowScrapeJob>(getImdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.title() == "The Simpsons");
        CHECK(show.firstAired() == QDate(1989, 12, 17));
        CHECK(show.ratings().size() == 1);
        CHECK(show.sortTitle().isEmpty());
        CHECK(show.actors().isEmpty());
    }

    SECTION("Loads all details for Scrubs")
    {
        ImdbTv tvdb;
        ShowScrapeJob::Config config{ShowIdentifier("tt0285403"), Locale("en-US"), tvdb.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<ImdbTvShowScrapeJob>(getImdbApi(), config);
        scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.imdbId() == ImdbId("tt0285403"));
        CHECK(show.title() == "Scrubs");
        CHECK(show.certification() == Certification("TV-14"));
        CHECK(show.firstAired() == QDate(2001, 10, 2));
        CHECK_THAT(show.overview(), StartsWith("Scrubs is a TV series starring Zach Braff"));

        REQUIRE_FALSE(show.ratings().isEmpty());
        CHECK(show.ratings().first().rating == Approx(8.3).margin(0.5));
        CHECK(show.ratings().first().voteCount > 3000);

        CHECK(show.runtime() == 22min);
        CHECK(show.posters().size() == 1);

        const auto& genres = show.genres();
        REQUIRE(genres.size() > 1);
        CHECK_THAT(genres[0], Contains("Comedy"));

        const auto& tags = show.tags();
        REQUIRE(tags.size() > 4);
        CHECK_THAT(tags, Contains("bromance"));
    }

    // Other languages are not yet supported. Reason:
    // IMDb filters based on IP. Only if users have an account, changing the language is possible.
    //
    // SECTION("Loads all details for Scrubs in another Language")
    // {
    //     ImdbTv tvdb;
    //     ShowScrapeJob::Config config{ShowIdentifier("tt0285403"), Locale("de-DE"), tvdb.meta().supportedShowDetails};

    //     auto scrapeJob = std::make_unique<ImdbTvShowScrapeJob>(getImdbApi(), config);
    //     scrapeTvScraperSync(scrapeJob.get());
    //     auto& show = scrapeJob->tvShow();

    //     CHECK(show.imdbId() == ImdbId("tt0285403"));

    //     CHECK(show.title() == "Scrubs");
    //     CHECK(show.certification() == Certification("TV-14"));
    //     CHECK(show.firstAired() == QDate(2001, 10, 2));
    //     CHECK_THAT(show.overview(),
    //         StartsWith("Mittelpunkt und Ich-Erzähler der Serie ist der junge Mediziner John Michael Dorian"));

    //     REQUIRE_FALSE(show.ratings().isEmpty());
    //     CHECK(show.ratings().first().rating == Approx(8.3).margin(0.5));
    //     CHECK(show.ratings().first().voteCount > 3000);

    //     CHECK(show.runtime() == 22min);

    //     const auto& genres = show.genres();
    //     REQUIRE(genres.size() > 0);
    //     CHECK_THAT(genres[0], Contains("Comedy"));
    // }
}
