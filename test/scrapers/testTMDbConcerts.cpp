#include "test/test_helpers.h"

#include "scrapers/concert/TMDbConcerts.h"
#include "settings/Settings.h"

#include <chrono>

using namespace std::chrono_literals;

/**
 * @brief Loads movie data synchronously
 */
template<class ScraperInterfaceT>
static void
loadConcertDataSync(ScraperInterfaceT& scraper, TmdbId ids, Concert& concert, QVector<ConcertScraperInfos> infos)
{
    QEventLoop loop;
    QEventLoop::connect(concert.controller(), &ConcertController::sigInfoLoadDone, &loop, &QEventLoop::quit);
    scraper.loadData(ids, &concert, infos);
    loop.exec();
}

TEST_CASE("TMDbConcert returns valid search results", "[scraper][TMDbConcert][search][requires_internet]")
{
    TMDbConcerts TMDb;

    SECTION("Search by concert name returns correct results")
    {
        const auto scraperResults = searchScraperSync(TMDb, "Rammstein in Amerika");
        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].name == "Rammstein in Amerika");
    }
}

TEST_CASE("TMDbConcert scrapes correct concert details", "[scraper][TMDbConcert][load_data][requires_internet]")
{
    TMDbConcerts tmdb;
    Settings::instance()->setUsePlotForOutline(true);

    SECTION("'Normal' concert loaded")
    {
        Concert m(QStringList{}); // Movie without files
        loadConcertDataSync(tmdb, TmdbId("361631"), m, tmdb.scraperSupports());

        REQUIRE(m.imdbId() == ImdbId("tt5053508"));
        CHECK(m.tmdbId() == TmdbId("361631"));

        CHECK(m.name() == "Rammstein in Amerika");
        CHECK(m.certification() == Certification::NoCertification);
        CHECK(m.released().toString("yyyy-MM-dd") == "2015-09-25");
        // Finding Dory has a user score of 69% (date: 2018-08-31)
        REQUIRE(!m.ratings().isEmpty());
        CHECK(m.ratings().first().rating == Approx(8.0).margin(0.5));
        CHECK(m.runtime() == 100min);

        CHECK_THAT(m.overview(), StartsWith("The concert film celebrates the band’s legendary show in New York’s"));

        const auto genres = m.genres();
        REQUIRE(genres.size() >= 2);
        CHECK(genres[0] == "Documentary");
        CHECK(genres[1] == "Music");
    }
}
