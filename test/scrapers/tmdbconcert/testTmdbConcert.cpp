#include "test/test_helpers.h"

#include "scrapers/concert/tmdb/TmdbConcert.h"
#include "scrapers/concert/tmdb/TmdbConcertSearchJob.h"
#include "settings/Settings.h"
#include "test/scrapers/testScraperHelpers.h"
#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

/**
 * @brief Loads movie data synchronously
 */
template<class ScraperInterfaceT>
static void
loadConcertDataSync(ScraperInterfaceT& scraper, TmdbId ids, Concert& concert, QSet<ConcertScraperInfo> infos)
{
    QEventLoop loop;
    QEventLoop::connect(concert.controller(), &ConcertController::sigInfoLoadDone, &loop, &QEventLoop::quit);
    scraper.loadData(ids, &concert, infos);
    loop.exec();
}

TEST_CASE("TmdbConcert returns valid search results", "[TmdbConcert][search]")
{
    SECTION("Search by concert name returns correct results")
    {
        ConcertSearchJob::Config config{"Rammstein in Amerika", Locale::English};
        auto* searchJob = new TmdbConcertSearchJob(getTmdbApi(), config);
        const auto scraperResults = searchConcertScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Rammstein in Amerika");
        CHECK(scraperResults[0].identifier.str() == "361631");
        CHECK(scraperResults[0].released == QDate(2015, 9, 25));
    }
}

TEST_CASE("TmdbConcert scrapes correct concert details", "[TmdbConcert][load_data]")
{
    TmdbConcert tmdb;
    Settings::instance()->setUsePlotForOutline(true);

    SECTION("'Normal' concert loaded")
    {
        Concert m(QStringList{}); // Movie without files
        loadConcertDataSync(tmdb, TmdbId("361631"), m, tmdb.scraperSupports());

        REQUIRE(m.imdbId() == ImdbId("tt5053508"));
        CHECK(m.tmdbId() == TmdbId("361631"));

        CHECK(m.title() == "Rammstein in Amerika");
        CHECK(m.certification() == Certification::NoCertification);
        CHECK(m.released().toString("yyyy-MM-dd") == "2015-09-25");
        // Finding Dory has a user score of 69% (date: 2018-08-31)
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().first().rating == Approx(8.0).margin(0.5));
        CHECK(m.runtime() == 223min);

        CHECK_THAT(m.overview(), StartsWith("The concert film celebrates the band’s legendary show in New York’s"));

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Documentary");
        CHECK(genres[1] == "Music");
    }
}
