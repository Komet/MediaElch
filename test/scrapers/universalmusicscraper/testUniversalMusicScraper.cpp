#include "test/test_helpers.h"

#include "scrapers/music/AllMusic.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

// static void scrapeAlbumSync()
// {
//    QEventLoop loop;
//    QEventLoop::connect(scrapeJob, &SeasonScrapeJob::sigFinished, [&](SeasonScrapeJob* /*unused*/) {
//        CAPTURE(scrapeJob->error().message);
//        REQUIRE(!scrapeJob->hasError());
//        loop.quit();
//    });
//    scrapeJob->start();
//    loop.exec();
// }

TEST_CASE("UniversalMusicScraper", "[music][UniversalMusicScraper][load_data]")
{
    SECTION("Search Album")
    {
        // TODO
        CHECK(true);
        //        REQUIRE(episode.tmdbId() == tmdbId);
        //        CHECK_THAT(episode.overview(), StartsWith("Ned empfindet für die Sängerin Rachel romantische
        //        Gefühle."));
    }
}
