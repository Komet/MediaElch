#include "test/test_helpers.h"

#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h"
#include "test/scrapers/imdbtv/testImdbTvHelper.h"
#include "tv_shows/TvShowEpisode.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

static void scrapeEpisodeSync(EpisodeScrapeJob* scrapeJob)
{
    QEventLoop loop;
    QEventLoop::connect(scrapeJob, &EpisodeScrapeJob::sigFinished, [&](EpisodeScrapeJob* /*unused*/) {
        CAPTURE(scrapeJob->error().message);
        REQUIRE(!scrapeJob->hasError());
        loop.quit();
    });
    scrapeJob->execute();
    loop.exec();
}

TEST_CASE("ImdbTv scrapes episode details for The Simpsons S12E19", "[episode][ImdbTv][load_data]")
{
    // Correct details for the episode
    QString episodeTitle = "I'm Goin' to Praiseland";
    SeasonNumber season(12);
    EpisodeNumber episodeNumber(19);
    ImdbId showId("tt0096697");
    ImdbId imdbId("tt0701133");

    const auto checkCommonFields = [&](TvShowEpisode& episode) {
        // Title is requested, ID is always set.
        CHECK(episode.imdbId() == imdbId);
        CHECK(episode.firstAired() == QDate(2001, 05, 06));
    };

    SECTION("Loads minimal details with episode ImdbTv ID")
    {
        EpisodeIdentifier id(imdbId);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        checkCommonFields(episode);
        CHECK(episode.title() == episodeTitle);
        // These fields should not be set
        CHECK(episode.actors().isEmpty());
    }

    SECTION("Loads minimal details with season and episode number")
    {
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        checkCommonFields(episode);
        CHECK(episode.title() == episodeTitle);
        // These fields should not be set
        CHECK(episode.actors().isEmpty());
    }

    SECTION("Loads all details for The Simpsons S12E19")
    {
        ImdbTv imdbTv;
        EpisodeIdentifier id(imdbId);
        EpisodeScrapeJob::Config config{id, Locale::English, imdbTv.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<ImdbTvEpisodeScrapeJob>(getImdbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        checkCommonFields(episode);
        // TODO
    }
}
