#include "test/test_helpers.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "scrapers/music/TvTunes.h"
#include "test/scrapers/universalmusicscraper/musicScraperUtils.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

using namespace mediaelch;

static QVector<ScraperSearchResult> loadTvTunesDataSync(scraper::TvTunes& scraper, const QString& searchStr)
{
    QVector<ScraperSearchResult> results;
    QEventLoop loop;
    QEventLoop::connect(
        &scraper, &scraper::TvTunes::sigSearchDone, &loop, [&results, &loop](QVector<ScraperSearchResult> resultList) {
            results = std::move(resultList);
            loop.quit();
        });
    scraper.search(searchStr);
    loop.exec();
    return results;
}

TEST_CASE("TvTunes search", "[music][TvTunes][search]")
{
    scraper::TvTunes tvTunes;
    QVector<ScraperSearchResult> results;

    results = loadTvTunesDataSync(tvTunes, "The Simpsons");
    REQUIRE(results.size() >= 40); // we limited to 50 max; but Simpsons has slightly fewer
    CHECK(results.first().name == "The Simpsons");
    CHECK(results.first().id == "https://www.televisiontunes.com/song/download/4780");

    results = loadTvTunesDataSync(tvTunes, "American Dad");
    REQUIRE(results.size() >= 20);
    CHECK(results.first().name == "American Dad");
    CHECK(results.first().id == "https://www.televisiontunes.com/song/download/38");
}
