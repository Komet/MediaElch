#include "test/test_helpers.h"

#include "music/Album.h"
#include "music/Artist.h"
#include "scrapers/music/AllMusic.h"
#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;

// Note: The MusicScrapers don't work independently, yet.
//       That's why we scraper the API page manually.

static QString downloadSyncOrFail(const QUrl& url)
{
    bool finished = false;
    QString result;
    QEventLoop loop;


    mediaelch::network::NetworkManager m_network;

    QNetworkRequest request(url);
    request.setRawHeader(
        "User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10; rv:33.0) Gecko/20100101 Firefox/33.0");
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
    SECTION("Load Artist Biography")
    {
        Artist artist;
        mediaelch::scraper::AllMusicApi api;
        mediaelch::scraper::AllMusic allmusic;

        AllMusicId id{"no-doubt-mn0000341672"};
        const auto url = api.makeArtistBiographyUrl(id);
        QString html = downloadSyncOrFail(url);

        REQUIRE_FALSE(html.isEmpty());

        allmusic.parseAndAssignArtistBiography(html, &artist, QSet<MusicScraperInfo>{MusicScraperInfo::Biography});

        CHECK_THAT(artist.biography(), StartsWith("Chart-topping new wave"));
    }
}
