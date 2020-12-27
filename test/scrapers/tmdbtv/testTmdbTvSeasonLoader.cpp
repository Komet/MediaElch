#include "test/test_helpers.h"

#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.h"
#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"
#include "tv_shows/TvShowEpisode.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

static void scrapeSeasonSync(SeasonScrapeJob* scrapeJob)
{
    QEventLoop loop;
    QEventLoop::connect(scrapeJob, &SeasonScrapeJob::sigFinished, [&](SeasonScrapeJob* /*unused*/) {
        CAPTURE(scrapeJob->error().message);
        REQUIRE(!scrapeJob->hasError());
        loop.quit();
    });
    scrapeJob->execute();
    loop.exec();
}

TEST_CASE("TmdbTv scrapes episode details for The Simpsons Season 12", "[season][TmdbTv][load_data]")
{
    waitForTmdbTvInitialized();

    // Correct details for the season
    SeasonNumber season(12);
    TmdbId showId("456");

    QString episodeTitle_s12e19 = "I'm Goin' to Praiseland";
    TmdbId tmdbId_s12e19("62494"); // TODO: Not useful at all?
    TvDbId tvdbId_s12e19("55719");
    ImdbId imdbId_s12e19("tt0701133");

    const auto checkCommonFields = [&](const TvShowEpisode& episode) {
        // Title is requested, IDs are always set.
        // TODO: CHECK(episode.tvdbId() == tvdbId_s12e19);
        // TODO: CHECK(episode.imdbId() == imdbId_s12e19);
        CHECK(episode.tmdbId() == tmdbId_s12e19);
        CHECK(episode.firstAired() == QDate(2001, 05, 06));
        // TODO: CHECK_THAT(episode.directors(), Contains("Chuck Sheetz"));
        // TODO: CHECK_THAT(episode.writers(), Contains("Julie Thacker"));
        CHECK(episode.ratings().first().rating == Approx(7.6).margin(0.2));
    };

    SECTION("Loads minimal episode details for specific season")
    {
        SeasonScrapeJob::Config config{ShowIdentifier(showId),
            Locale::English,
            {SeasonNumber(12)},
            SeasonOrder::Aired,
            {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvSeasonScrapeJob>(getTmdbApi(), config);
        scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        CHECK(episodes.size() == 21); // Season 12 is scraped and has all seasons

        const auto* episode = episodes[{SeasonNumber(12), EpisodeNumber(19)}];
        REQUIRE(episode != nullptr);
        checkCommonFields(*episode);
        CHECK(episode->title() == episodeTitle_s12e19);
        CHECK_THAT(episode->overview(), StartsWith("After finding a sketchbook belonging to his late wife Maude"));
        // TODO: CHECK(episode->actors().size() >= 10);
    }

    SECTION("Loads minimal episode details for all seasons")
    {
        SeasonScrapeJob::Config config{
            ShowIdentifier(showId), Locale::English, {}, SeasonOrder::Aired, {EpisodeScraperInfo::Title}};

        auto scrapeJob = std::make_unique<TmdbTvSeasonScrapeJob>(getTmdbApi(), config);
        scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        CHECK(episodes.size() >= 750); // There are >30 seasons

        const auto* episode = episodes[{SeasonNumber(12), EpisodeNumber(19)}];
        REQUIRE(episode != nullptr);
        checkCommonFields(*episode);
        CHECK(episode->title() == episodeTitle_s12e19);
        CHECK_THAT(episode->overview(), StartsWith("After finding a sketchbook belonging to his late wife Maude"));
        // TODO: CHECK(episode->actors().size() >= 10);
    }
}
