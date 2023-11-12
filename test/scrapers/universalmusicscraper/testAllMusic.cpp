#include "test/test_helpers.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "scrapers/music/AllMusic.h"
#include "test/scrapers/universalmusicscraper/musicScraperUtils.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

using namespace mediaelch;

// Note: The MusicScrapers don't work independently, yet.
//       That's why we scrape the API page manually.


TEST_CASE("AllMusic", "[music][AllMusic][load_data]")
{
    QSet<MusicScraperInfo> allDetails = mediaelch::allMusicScraperInfos();
    mediaelch::scraper::AllMusicApi api;
    mediaelch::scraper::AllMusic allmusic;

    SECTION("Load Artist (Person) Details for living artist")
    {
        AllMusicId id{"paloma-faith-mn0002337416"};
        Artist artist;

        const auto url = api.makeArtistUrl(id);
        QString html = test::musicDownloadSyncOrFail(url);

        allmusic.parseAndAssignArtist(html, artist, allDetails);

        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/paloma-faith-details-mn0000341672");
    }

    SECTION("Load Artist (Person) Biography for dead artist")
    {
        AllMusicId id{"falco-mn0000793845"};
        Artist artist;

        const auto url = api.makeArtistBiographyUrl(id);
        QString html = test::musicDownloadSyncOrFail(url, api.makeArtistUrl(id));

        allmusic.parseAndAssignArtistBiography(html, artist, allDetails);

        REQUIRE_THAT(artist.biography(), StartsWith("Falco Biography by"));
        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/falco-biography-mn0000793845");
    }

    SECTION("Load Artist (Person) Moods for dead artist")
    {
        AllMusicId id{"falco-mn0000793845"};
        Artist artist;

        const auto url = api.makeArtistMoodsUrl(id);
        QString html = test::musicDownloadSyncOrFail(url, api.makeArtistUrl(id));

        allmusic.parseAndAssignArtistMoods(html, artist, allDetails);

        REQUIRE_FALSE(artist.moods().isEmpty());
        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/falco-moods-mn0000793845");
    }

    SECTION("Load Artist (Person) Details for dead artist")
    {
        AllMusicId id{"falco-mn0000793845"};
        Artist artist;

        const auto url = api.makeArtistUrl(id);
        QString html = test::musicDownloadSyncOrFail(url);

        allmusic.parseAndAssignArtist(html, artist, allDetails);

        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/falco-details-mn0000793845");
    }

    SECTION("Load Artist (Band) Details")
    {
        AllMusicId id{"no-doubt-mn0000341672"};
        Artist artist;

        const auto url = api.makeArtistUrl(id);
        QString html = test::musicDownloadSyncOrFail(url);

        allmusic.parseAndAssignArtist(html, artist, allDetails);
        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/no-doubt-details-mn0000341672");
    }

    SECTION("Load Artist (Band) Biography")
    {
        AllMusicId id{"no-doubt-mn0000341672"};
        Artist artist;

        const auto url = api.makeArtistBiographyUrl(id);
        QString html = test::musicDownloadSyncOrFail(url, api.makeArtistUrl(id));

        allmusic.parseAndAssignArtistBiography(html, artist, allDetails);

        REQUIRE_THAT(artist.biography(), StartsWith("No Doubt Biography"));
        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/no-doubt-biography-mn0000341672");
    }

    SECTION("Load Artist (Band) Moods")
    {
        AllMusicId id{"no-doubt-mn0000341672"};
        Artist artist;

        const auto url = api.makeArtistMoodsUrl(id);
        QString html = test::musicDownloadSyncOrFail(url, api.makeArtistUrl(id));

        allmusic.parseAndAssignArtistMoods(html, artist, allDetails);

        REQUIRE_FALSE(artist.moods().isEmpty());
        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/no-doubt-moods-mn0000341672");
    }
}
