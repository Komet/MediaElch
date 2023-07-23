#include "test/test_helpers.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "data/music/ArtistController.h"
#include "scrapers/music/MusicBrainz.h"
#include "test/scrapers/universalmusicscraper/musicScraperUtils.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

using namespace mediaelch;

namespace test {

static QString
loadMusicBrainzArtistSync(mediaelch::scraper::MusicBrainzApi& api, const Locale& locale, MusicBrainzId id)
{
    CAPTURE(id);
    QString artistData;
    QEventLoop loop;
    api.loadArtist(locale, id, [&artistData, &loop](QString data, ScraperError error) {
        CAPTURE(error);
        REQUIRE_FALSE(error.hasError());

        REQUIRE_FALSE(data.isEmpty());
        artistData = data;
        loop.quit();
    });
    loop.exec();
    return artistData;
}

static QString loadMusicBrainzAlbumSync(mediaelch::scraper::MusicBrainzApi& api, const Locale& locale, MusicBrainzId id)
{
    CAPTURE(id);
    QString albumData;
    QEventLoop loop;
    api.loadAlbum(locale, id, [&albumData, &loop](QString data, ScraperError error) {
        CAPTURE(error);
        REQUIRE_FALSE(error.hasError());

        REQUIRE_FALSE(data.isEmpty());
        albumData = data;
        loop.quit();
    });
    loop.exec();
    return albumData;
}

static QString
loadMusicBrainzReleaseGroupSync(mediaelch::scraper::MusicBrainzApi& api, const Locale& locale, MusicBrainzId id)
{
    CAPTURE(id);
    QString releaseGroupData;
    QEventLoop loop;
    api.loadReleaseGroup(locale, id, [&releaseGroupData, &loop](QString data, ScraperError error) {
        CAPTURE(error);
        REQUIRE_FALSE(error.hasError());

        REQUIRE_FALSE(data.isEmpty());
        releaseGroupData = data;
        loop.quit();
    });
    loop.exec();
    return releaseGroupData;
}

static QString
musicBrainzGetSync(const mediaelch::Locale& locale, mediaelch::scraper::MusicBrainzApi& api, const QUrl& url)
{
    CAPTURE(url);
    QString response;
    QEventLoop loop;

    api.sendGetRequest(locale, url, [&response, &loop](QString result, ScraperError error) {
        CAPTURE(error);
        REQUIRE_FALSE(error.hasError());

        REQUIRE_FALSE(result.isEmpty());
        response = result;
        loop.quit();
    });

    loop.exec();
    return response;
}

} // namespace test

// Note: The MusicScrapers don't work independently, yet.
//       That's why we scrape the API page manually.

TEST_CASE("MusicBrainz search", "[music][MusicBrainz][search]")
{
    QString metallicaArtistId{"65f4f0c5-ef9e-490c-aee3-909e7ae6b2ab"};
    QSet<MusicScraperInfo> allDetails = mediaelch::allMusicScraperInfos();
    mediaelch::scraper::MusicBrainzApi musicBrainzApi;
    mediaelch::scraper::MusicBrainz musicBrainz;

    SECTION("search artist")
    {
        // This URL is scraped from MusicBrainz.
        QUrl searchUrl = musicBrainzApi.makeArtistSearchUrl("Metallica");
        CAPTURE(searchUrl);

        QString html = test::musicBrainzGetSync(mediaelch::Locale::English, musicBrainzApi, searchUrl);
        REQUIRE_FALSE(html.isEmpty());

        QVector<ScraperSearchResult> results = musicBrainzApi.parseArtistSearchPage(html);
        REQUIRE_FALSE(results.isEmpty());

        CHECK(results.size() > 2);
        CHECK(results.first().name == QStringLiteral("Metallica"));
        CHECK(results.first().id == metallicaArtistId);
    }

    SECTION("search album")
    {
        // This URL is scraped from MusicBrainz.
        QUrl searchUrl = musicBrainzApi.makeAlbumSearchUrl("Master of Puppets");
        CAPTURE(searchUrl);

        QString html = test::musicBrainzGetSync(mediaelch::Locale::English, musicBrainzApi, searchUrl);
        REQUIRE_FALSE(html.isEmpty());

        QVector<ScraperSearchResult> results = musicBrainzApi.parseAlbumSearchPage(html);
        REQUIRE_FALSE(results.isEmpty());

        CHECK(results.size() > 20);
        CHECK_THAT(results.first().name, StartsWith("Master of Puppets"));
    }

    SECTION("search album for artist")
    {
        // This URL is scraped from MusicBrainz.
        QUrl searchUrl = musicBrainzApi.makeAlbumWithArtistSearchUrl("Master of Puppets", "Metallica");
        CAPTURE(searchUrl);

        QString html = test::musicBrainzGetSync(mediaelch::Locale::English, musicBrainzApi, searchUrl);
        REQUIRE_FALSE(html.isEmpty());

        QVector<ScraperSearchResult> results = musicBrainzApi.parseAlbumSearchPage(html);
        REQUIRE_FALSE(results.isEmpty());

        CHECK(results.size() > 20);
        CHECK_THAT(results.first().name, StartsWith("Master of Puppets"));
    }
}


TEST_CASE("MusicBrainz load", "[music][MusicBrainz][load_data]")
{
    QSet<MusicScraperInfo> allDetails = mediaelch::allMusicScraperInfos();

    scraper::MusicBrainzApi api;
    scraper::MusicBrainz musicBrainz;
    Locale locale = Locale::English;

    MusicBrainzId metallicaId{"65f4f0c5-ef9e-490c-aee3-909e7ae6b2ab"};

    SECTION("load artist")
    {
        QString artistStr = test::loadMusicBrainzArtistSync(api, locale, metallicaId);
        CHECK_THAT(artistStr, Contains("65f4f0c5-ef9e-490c-aee3-909e7ae6b2ab"));
        CHECK_THAT(artistStr, Contains("<artist"));

        // loadArtist() is only used to get further IDs, no actual metadata is loaded.
    }

    SECTION("load artist biography")
    {
        QUrl biographyUrl = api.makeArtistBiographyUrl(metallicaId);
        QString json = test::musicBrainzGetSync(locale, api, biographyUrl);
        Artist artist;
        musicBrainz.parseAndAssignArtist(json, artist, allDetails);
        test::scraper::compareAgainstReference(artist, "scrapers/musicbrainz/metallica-biography-english");
    }

    SECTION("load artist biography in German")
    {
        // https://musicbrainz.org/artist/65f4f0c5-ef9e-490c-aee3-909e7ae6b2ab/wikipedia-extract
        QUrl biographyUrl = api.makeArtistBiographyUrl(metallicaId);
        QString json = test::musicBrainzGetSync(mediaelch::Locale("de-DE"), api, biographyUrl);
        Artist artist;
        musicBrainz.parseAndAssignArtist(json, artist, allDetails);
        test::scraper::compareAgainstReference(artist, "scrapers/musicbrainz/metallica-biography-german");
    }

    SECTION("load artist biography in French")
    {
        // https://musicbrainz.org/artist/65f4f0c5-ef9e-490c-aee3-909e7ae6b2ab/wikipedia-extract
        QUrl biographyUrl = api.makeArtistBiographyUrl(metallicaId);
        QString json = test::musicBrainzGetSync(mediaelch::Locale("fr"), api, biographyUrl);
        Artist artist;
        musicBrainz.parseAndAssignArtist(json, artist, allDetails);
        test::scraper::compareAgainstReference(artist, "scrapers/musicbrainz/metallica-biography-french");
    }

    SECTION("load album")
    {
        // Master of Puppets by Metallica
        MusicBrainzId id{"fed37cfc-2a6d-4569-9ac0-501a7c7598eb"};
        QString albumStr = test::loadMusicBrainzAlbumSync(api, locale, id);
        CHECK_THAT(albumStr, Contains("fed37cfc-2a6d-4569-9ac0-501a7c7598eb"));
        CHECK_THAT(albumStr, Contains("<release "));

        Album album;
        musicBrainz.parseAndAssignAlbum(albumStr, album, allDetails);

        test::scraper::compareAgainstReference(album, "scrapers/musicbrainz/master-of-puppets-album-details");
    }

    SECTION("load release group")
    {
        // Master of Puppets release group (no specific album).
        // Release groups are loaded if no album can be loaded.
        MusicBrainzId id{"3d00fb45-f8ab-3436-a8e1-b4bfc4d66913"};
        QString albumStr = test::loadMusicBrainzReleaseGroupSync(api, locale, id);
        CHECK_THAT(albumStr, Contains("3d00fb45-f8ab-3436-a8e1-b4bfc4d66913"));
        CHECK_THAT(albumStr, Contains("<release-group "));

        Album album;
        musicBrainz.parseAndAssignAlbum(albumStr, album, allDetails);

        test::scraper::compareAgainstReference(album, "scrapers/musicbrainz/master-of-puppets-release-group-details");
    }
}
