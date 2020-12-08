#include "test/test_helpers.h"

#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/thetvdb/TheTvDbEpisodeScrapeJob.h"
#include "test/scrapers/thetvdb/testTheTvDbHelper.h"
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

TEST_CASE("TheTvDb scrapes episode details for The Simpsons S12E19", "[episode][TheTvDb][load_data]")
{
    waitForTheTvDbInitialized();

    // Correct details for the episode
    QString episodeTitle = "I'm Goin' to Praiseland";
    SeasonNumber season(12);
    EpisodeNumber episodeNumber(19);
    TvDbId showId("71663");
    TvDbId tvdbId("55719");
    ImdbId imdbId("tt0701133");

    const auto checkCommonFields = [&](TvShowEpisode& episode) {
        // Title is requested, IDs are always set.
        CHECK(episode.tvdbId() == tvdbId);
        CHECK(episode.imdbId() == imdbId);
        CHECK(episode.firstAired() == QDate(2001, 05, 06));
    };

    SECTION("Loads minimal details with episode TheTvDb ID")
    {
        EpisodeIdentifier id(tvdbId);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TheTvDbEpisodeScrapeJob>(getTheTvDbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        checkCommonFields(episode);
        CHECK(episode.title() == episodeTitle);
    }

    SECTION("Loads minimal details with season and episode number")
    {
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TheTvDbEpisodeScrapeJob>(getTheTvDbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        checkCommonFields(episode);
        CHECK(episode.title() == episodeTitle);
    }

    SECTION("Loads minimal details for The Simpsons S12E19 in other language")
    {
        EpisodeIdentifier id(tvdbId);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TheTvDbEpisodeScrapeJob>(getTheTvDbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        CHECK(episode.title() == "Wunder gibt es immer wieder");
        checkCommonFields(episode);
    }

    SECTION("Loads all details for The Simpsons S12E19")
    {
        // TODO: Signal muss mit timer geschlossen werden , also sigFinished
        TheTvDb tvdb;
        EpisodeIdentifier id(tvdbId);
        EpisodeScrapeJob::Config config{id, Locale::English, tvdb.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TheTvDbEpisodeScrapeJob>(getTheTvDbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        checkCommonFields(episode);

        // TODO
    }

    SECTION("Loads all details for The Simpsons S12E19 in another Language")
    {
        TheTvDb tvdb;
        EpisodeIdentifier id(tvdbId);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), tvdb.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TheTvDbEpisodeScrapeJob>(getTheTvDbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        checkCommonFields(episode);

        // TODO
    }
}

TEST_CASE("TheTvDb scrapes episode details and respects DVD/Aired order", "[episode][TheTvDb][load_data]")
{
    TvDbId spaceId("76366");
    SeasonNumber season(1);
    EpisodeNumber episodeNumber(2);

    SECTION("Loads all details for 'Space: 1999' S01E02 in DVD order")
    {
        EpisodeIdentifier id(spaceId.toString(), season, episodeNumber, SeasonOrder::Dvd);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};
        auto scrapeJob = std::make_unique<TheTvDbEpisodeScrapeJob>(getTheTvDbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        CHECK(episode.title() == "Matter of Life and Death");
    }

    SECTION("Loads all details for 'Space: 1999' S01E02 in Aired order")
    {
        EpisodeIdentifier id(spaceId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale::English, {EpisodeScraperInfo::Title}};
        auto scrapeJob = std::make_unique<TheTvDbEpisodeScrapeJob>(getTheTvDbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        CHECK(episode.title() == "Force of Life");
    }
}
