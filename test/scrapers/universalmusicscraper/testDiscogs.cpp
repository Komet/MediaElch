#include "test/test_helpers.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "scrapers/music/Discogs.h"
#include "test/scrapers/universalmusicscraper/musicScraperUtils.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

using namespace mediaelch;

// Note: The MusicScrapers don't work independently, yet.
//       That's why we scrape the API page manually.


TEST_CASE("Discogs", "[music][Discogs][load_data]")
{
    QSet<MusicScraperInfo> allDetails = mediaelch::allMusicScraperInfos();
    mediaelch::scraper::Discogs discogs;

    SECTION("Load Artist (Band) Details")
    {
        // This URL is scraped from MusicBrainz.
        QUrl discogsMetallicaArtistPage{"https://www.discogs.com/artist/18839?type=Releases&subtype=Albums"};
        CAPTURE(discogsMetallicaArtistPage);

        Artist artist;
        QString html = test::downloadSyncOrFail(discogsMetallicaArtistPage);

        REQUIRE_FALSE(html.isEmpty());

        discogs.parseAndAssignArtist(html, &artist, allDetails);

        test::scraper::compareAgainstReference(artist, "scrapers/discogs/metallica-details-18839");
    }

    SECTION("Load Artist (Band) Details from German Discogs")
    {
        // Note: Currently not used, because the URL is scraped from MusicBrainz.
        QUrl discogsMetallicaArtistPage{"https://www.discogs.com/de/artist/18839?type=Releases&subtype=Albums"};
        CAPTURE(discogsMetallicaArtistPage);

        Artist artist;
        QString html = test::downloadSyncOrFail(discogsMetallicaArtistPage);

        REQUIRE_FALSE(html.isEmpty());

        discogs.parseAndAssignArtist(html, &artist, allDetails);

        test::scraper::compareAgainstReference(artist, "scrapers/discogs/metallica-details-german-18839");
    }

    SECTION("Load Album Details")
    {
        // This URL is scraped from MusicBrainz.
        QUrl discogsMasterOfPuppetsAlbumPage{"https://www.discogs.com/master/6495"};
        CAPTURE(discogsMasterOfPuppetsAlbumPage);

        Album album;
        QString html = test::downloadSyncOrFail(discogsMasterOfPuppetsAlbumPage);

        REQUIRE_FALSE(html.isEmpty());

        discogs.parseAndAssignAlbum(html, &album, allDetails);

        test::scraper::compareAgainstReference(album, "scrapers/discogs/master-of-puppets-details-6495");
    }

    SECTION("Load Album Details from German Discogs")
    {
        // This URL is scraped from MusicBrainz.
        QUrl discogsMasterOfPuppetsAlbumPage{"https://www.discogs.com/de/master/6495"};
        CAPTURE(discogsMasterOfPuppetsAlbumPage);

        Album album;
        QString html = test::downloadSyncOrFail(discogsMasterOfPuppetsAlbumPage);

        REQUIRE_FALSE(html.isEmpty());

        discogs.parseAndAssignAlbum(html, &album, allDetails);

        test::scraper::compareAgainstReference(album, "scrapers/discogs/master-of-puppets-details-german-6495");
    }
}
