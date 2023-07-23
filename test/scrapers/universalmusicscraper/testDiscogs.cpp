#include "test/test_helpers.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "scrapers/music/Discogs.h"
#include "test/helpers/scraper_helpers.h"
#include "test/scrapers/universalmusicscraper/musicScraperUtils.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("Discogs", "[music][Discogs][load_data]")
{
    mediaelch::scraper::Discogs discogs;

    SECTION("Load Artist (Band) Details: Metallica")
    {
        DiscogsArtistScrapeJob::Config config;
        config.identifier = "18839";
        config.details = allMusicScraperInfos();
        ArtistScrapeJob* scrapeJob = discogs.loadArtist(config);
        auto dls = makeDeleteLaterScope(scrapeJob);
        test::scrapeJobSync(scrapeJob);
        REQUIRE(scrapeJob->artist().name() == "Metallica");
        test::scraper::compareAgainstReference(scrapeJob->artist(), "scrapers/discogs/metallica-details-18839");
    }

    SECTION("Load Artist (Band) Details: Rammstein")
    {
        DiscogsArtistScrapeJob::Config config;
        config.identifier = "11771";
        config.details = allMusicScraperInfos();
        ArtistScrapeJob* scrapeJob = discogs.loadArtist(config);
        auto dls = makeDeleteLaterScope(scrapeJob);
        test::scrapeJobSync(scrapeJob);
        REQUIRE(scrapeJob->artist().name() == "Rammstein");
        test::scraper::compareAgainstReference(scrapeJob->artist(), "scrapers/discogs/rammstein-details-11771");
    }

    SECTION("Load Album Details")
    {
        DiscogsAlbumScrapeJob::Config config;
        config.identifier = "6495";
        config.details = allMusicScraperInfos();
        AlbumScrapeJob* scrapeJob = discogs.loadAlbum(config);
        auto dls = makeDeleteLaterScope(scrapeJob);
        test::scrapeJobSync(scrapeJob);
        REQUIRE(scrapeJob->album().title() == "Master Of Puppets");
        test::scraper::compareAgainstReference(scrapeJob->album(), "scrapers/discogs/master-of-puppets-details-6495");
    }
}
