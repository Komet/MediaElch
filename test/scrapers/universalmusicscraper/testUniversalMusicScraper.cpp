#include "test/test_helpers.h"

#include "scrapers/music/AllMusic.h"
#include "test/helpers/scraper_helpers.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("UniversalMusicScraper returns correct search results", "[music][UniversalMusicScraper][search]")
{
    UniversalMusicScraper scraper;

    SECTION("Search for Artist Rammstein")
    {
        ArtistSearchJob::Config config;
        config.locale = "en";
        config.query = "Rammstein";

        auto* searchJob = scraper.searchArtist(config);
        const auto scraperResults = test::searchArtistScraperSync(searchJob).first;

        REQUIRE(scraperResults.size() > 1);

        CHECK(scraperResults[0].title == "Rammstein");
        CHECK(scraperResults[0].identifier == "b2d122f9-eadb-4930-a196-8f221eeb0c66");
    }

    SECTION("Search for Album 'Rosenrot'")
    {
        AlbumSearchJob::Config config;
        config.locale = "en";
        config.artistName = "Rammstein";
        config.albumQuery = "Rosenrot";

        auto* searchJob = scraper.searchAlbum(config);
        const auto scraperResults = test::searchAlbumScraperSync(searchJob).first;

        REQUIRE(scraperResults.size() > 10);

        CHECK_THAT(scraperResults[0].title, StartsWith("Rosenrot"));
        CHECK(scraperResults[0].groupIdentifier == "15e65e08-85d1-3145-85b2-e18b9fbd4cba");
        CHECK(scraperResults[0].identifier == "f3bfed3d-c1d2-3599-b6d7-3916be6c53dc");

        CHECK_THAT(scraperResults[1].title, StartsWith("Rosenrot"));
        CHECK(scraperResults[1].groupIdentifier == "7bfae355-44ea-3be6-89de-40147131be51");
        CHECK(scraperResults[1].identifier == "241739af-fc56-46b1-bd42-af06fdbff62a");
    }
}

TEST_CASE("UniversalMusicScraper loads correct details", "[music][UniversalMusicScraper][load_data]")
{
    QSet<MusicScraperInfo> noImages = {
        MusicScraperInfo::Name,
        MusicScraperInfo::Genres,
        MusicScraperInfo::Styles,
        MusicScraperInfo::Moods,
        MusicScraperInfo::YearsActive,
        MusicScraperInfo::Formed,
        MusicScraperInfo::Born,
        MusicScraperInfo::Died,
        MusicScraperInfo::Disbanded,
        MusicScraperInfo::Biography,
        // MusicScraperInfo::Thumb,
        // MusicScraperInfo::Fanart,
        // MusicScraperInfo::Logo,
        MusicScraperInfo::Title,
        MusicScraperInfo::Artist,
        MusicScraperInfo::Review,
        MusicScraperInfo::ReleaseDate,
        MusicScraperInfo::Label,
        MusicScraperInfo::Rating,
        MusicScraperInfo::Year,
        // MusicScraperInfo::CdArt,
        // MusicScraperInfo::Cover,
        // MusicScraperInfo::ExtraFanarts,
        MusicScraperInfo::Discography,
    };

    SECTION("Load Artist Metallica")
    {
        QEventLoop loop;
        Artist artist;
        UniversalMusicScraper scraper;
        QEventLoop::connect(artist.controller(), &ArtistController::sigLoadDone, &loop, &QEventLoop::quit);
        MusicBrainzId metallica("65f4f0c5-ef9e-490c-aee3-909e7ae6b2ab");
        artist.controller()->loadData(metallica, &scraper, noImages);
        loop.exec();

        REQUIRE(artist.name() == "Metallica");
        test::scraper::compareAgainstReference(artist, "scrapers/universalmusicscraper/metallica");
    }

    SECTION("Load Album Metallica")
    {
        QEventLoop loop;
        Album album;
        UniversalMusicScraper scraper;
        QEventLoop::connect(album.controller(), &AlbumController::sigLoadDone, &loop, &QEventLoop::quit);
        // Website: https://musicbrainz.org/release-group/3d00fb45-f8ab-3436-a8e1-b4bfc4d66913
        // API:
        //   https://musicbrainz.org/ws/2/release-group/3d00fb45-f8ab-3436-a8e1-b4bfc4d66913?inc=url-rels+artist-credits
        MusicBrainzId masterOfPuppetsReleaseGroup("3d00fb45-f8ab-3436-a8e1-b4bfc4d66913");
        // Website:https://musicbrainz.org/release/fed37cfc-2a6d-4569-9ac0-501a7c7598eb
        // API:
        //   https://musicbrainz.org/ws/2/release/fed37cfc-2a6d-4569-9ac0-501a7c7598eb?inc=url-rels+labels+artist-credits
        MusicBrainzId masterOfPuppetsRelease("fed37cfc-2a6d-4569-9ac0-501a7c7598eb");
        album.controller()->loadData(masterOfPuppetsRelease, masterOfPuppetsReleaseGroup, &scraper, noImages);
        loop.exec();

        REQUIRE(album.title() == "Master of Puppets");
        test::scraper::compareAgainstReference(album, "scrapers/universalmusicscraper/master-of-puppets");
    }
}
