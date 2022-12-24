#include "test/test_helpers.h"

#include "scrapers/music/AllMusic.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

// static auto scrapeAlbumSync(mediaelch::scraper::MusicScraper* scraper) -> QVector<ScraperSearchResult>
// {
//    QVector<ScraperSearchResult> searchResults;
//    QEventLoop loop;
//    QEventLoop::connect(scraper, &MusicScraper::sigSearchDone, &loop, [&](QVector<ScraperSearchResult> results) {
//        searchResults = std::move(results);
//        loop.quit();
//    });
//    loop.exec();
//    return searchResults;
// }

TEST_CASE("UniversalMusicScraper", "[music][UniversalMusicScraper][load_data]")
{
    SECTION("Search Album")
    {
        // auto scraper = std::make_unique<UniversalMusicScraper>();
        // TODO
        CHECK(true);
        //        REQUIRE(episode.tmdbId() == tmdbId);
        //        CHECK_THAT(episode.overview(), StartsWith("Ned empfindet für die Sängerin Rachel romantische
        //        Gefühle."));
    }
}
