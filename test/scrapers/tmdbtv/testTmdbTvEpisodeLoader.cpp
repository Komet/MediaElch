#include "test/test_helpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.h"
#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"
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

TEST_CASE("TmdbTv scrapes episode details for The Simpsons S12E19", "[episode][TmdbTv][load_data]")
{
    waitForTmdbTvInitialized();

    // Correct details for the episode
    QString episodeTitle = "I'm Goin' to Praiseland";
    SeasonNumber season(12);
    EpisodeNumber episodeNumber(19);
    TmdbId showId("456");
    TvDbId tvdbId("55719");
    TmdbId tmdbId("62494"); // TODO: Not useful at all?
    ImdbId imdbId("tt0701133");

    const auto checkCommonFields = [&](TvShowEpisode& episode) {
        // Title is requested, IDs are always set.
        CHECK(episode.tvdbId() == tvdbId);
        CHECK(episode.tmdbId() == tmdbId);
        CHECK(episode.imdbId() == imdbId);
        CHECK(episode.firstAired() == QDate(2001, 05, 06));
        // TODO: CHECK_THAT(episode.directors(), Contains("Chuck Sheetz"));
        // TODO: CHECK_THAT(episode.writers(), Contains("Julie Thacker"));
        CHECK(episode.ratings().first().rating == Approx(7.6).margin(0.2));
    };

    SECTION("Loads minimal details with season and episode number")
    {
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("en-US"), {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvEpisodeScrapeJob>(getTmdbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();
        checkCommonFields(episode);
        CHECK(episode.title() == episodeTitle);

        CHECK(episode.actors().size() >= 10);
    }

    SECTION("Loads minimal details for The Simpsons S12E19 in other language")
    {
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvEpisodeScrapeJob>(getTmdbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        CHECK(episode.title() == "Wunder gibt es immer wieder");
        checkCommonFields(episode);

        CHECK(episode.actors().size() >= 10);
    }

    SECTION("Loads all details for The Simpsons S12E19")
    {
        // TODO: Signal muss mit timer geschlossen werden , also sigFinished
        TmdbTv tmdb;
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("en-US"), tmdb.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TmdbTvEpisodeScrapeJob>(getTmdbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        CHECK_THAT(episode.overview(), StartsWith("After finding a sketchbook belonging to his late wife Maude"));
        // TODO
    }

    SECTION("Loads all details for The Simpsons S12E19 in another Language")
    {
        TmdbTv tmdb;
        EpisodeIdentifier id(showId.toString(), season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), tmdb.meta().supportedEpisodeDetails};

        auto scrapeJob = std::make_unique<TmdbTvEpisodeScrapeJob>(getTmdbApi(), config);
        scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE(episode.tmdbId() == tmdbId);
        CHECK_THAT(episode.overview(), StartsWith("Ned empfindet für die Sängerin Rachel romantische Gefühle."));
    }
}
