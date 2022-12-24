#include "test/test_helpers.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "network/NetworkRequest.h"
#include "scrapers/music/AllMusic.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;

// Note: The MusicScrapers don't work independently, yet.
//       That's why we scrape the API page manually.

static QString downloadSyncOrFail(const QUrl& url)
{
    bool finished = false;
    QString result;
    QEventLoop loop;


    mediaelch::network::NetworkManager m_network;

    QNetworkRequest request(url);
    mediaelch::network::useFirefoxUserAgent(request);
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");

    QNetworkReply* reply = m_network.getWithWatcher(request);
    QEventLoop::connect(reply, &QNetworkReply::finished, &loop, [&]() {
        finished = true;
        CHECK(reply->error() == QNetworkReply::NoError);
        result = QString::fromUtf8(reply->readAll());
        loop.quit();
    });

    if (!finished) {
        loop.exec();
    }
    return result;
}

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
        QString html = downloadSyncOrFail(url);

        REQUIRE_FALSE(html.isEmpty());

        allmusic.parseAndAssignArtist(html, &artist, allDetails);

        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/aloma-faith-details-mn0000341672");
    }

    SECTION("Load Artist (Person) Biography for dead artist")
    {
        AllMusicId id{"falco-mn0000793845"};
        Artist artist;

        const auto url = api.makeArtistBiographyUrl(id);
        QString html = downloadSyncOrFail(url);

        REQUIRE_FALSE(html.isEmpty());

        allmusic.parseAndAssignArtistBiography(html, &artist, allDetails);

        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/falco-biography-mn0000793845");
    }

    SECTION("Load Artist (Person) Details for dead artist")
    {
        AllMusicId id{"falco-mn0000793845"};
        Artist artist;

        const auto url = api.makeArtistUrl(id);
        QString html = downloadSyncOrFail(url);

        REQUIRE_FALSE(html.isEmpty());

        allmusic.parseAndAssignArtist(html, &artist, allDetails);

        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/falco-details-mn0000793845");
    }

    SECTION("Load Artist (Band) Details")
    {
        AllMusicId id{"no-doubt-mn0000341672"};
        Artist artist;

        const auto url = api.makeArtistUrl(id);
        QString html = downloadSyncOrFail(url);

        REQUIRE_FALSE(html.isEmpty());

        allmusic.parseAndAssignArtist(html, &artist, allDetails);

        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/no-doubt-details-mn0000341672");
    }

    SECTION("Load Artist (Band) Biography")
    {
        AllMusicId id{"no-doubt-mn0000341672"};
        Artist artist;

        const auto url = api.makeArtistBiographyUrl(id);
        QString html = downloadSyncOrFail(url);

        REQUIRE_FALSE(html.isEmpty());

        allmusic.parseAndAssignArtistBiography(html, &artist, QSet<MusicScraperInfo>{MusicScraperInfo::Biography});

        REQUIRE_THAT(artist.biography(), StartsWith("Chart-topping new wave"));
        test::scraper::compareAgainstReference(artist, "scrapers/allmusic/no-doubt-biography-mn0000341672");
    }
}
