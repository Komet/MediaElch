#include "test/test_helpers.h"

#include "scrapers/concert/tmdb/TmdbConcert.h"
#include "scrapers/concert/tmdb/TmdbConcertSearchJob.h"
#include "settings/Settings.h"
#include "test/helpers/scraper_helpers.h"
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
        const auto scraperResults = test::searchConcertScraperSync(searchJob).first;

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

    SECTION("Rammstein in Amerika")
    {
        Concert concert(QStringList{}); // Movie without files
        loadConcertDataSync(tmdb, TmdbId("361631"), concert, tmdb.meta().supportedDetails);
        test::scraper::compareAgainstReference(concert, "scrapers/tmdb_concert/Rammstein_in_Amerika_tmdb361631");
    }

    SECTION("Katy Perry: MTV Unplugged")
    {
        Concert concert(QStringList{}); // Movie without files
        loadConcertDataSync(tmdb, TmdbId("167366"), concert, tmdb.meta().supportedDetails);
        test::scraper::compareAgainstReference(concert, "scrapers/tmdb_concert/Katy_Perry_MTV_Unplugged_tmdb167366");
    }
}
