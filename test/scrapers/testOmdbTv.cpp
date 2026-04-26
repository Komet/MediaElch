#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/omdb/OmdbTv.h"
#include "scrapers/tv_show/omdb/OmdbTvConfiguration.h"
#include "scrapers/tv_show/omdb/OmdbTvEpisodeScrapeJob.h"
#include "scrapers/tv_show/omdb/OmdbTvSeasonScrapeJob.h"
#include "scrapers/tv_show/omdb/OmdbTvShowScrapeJob.h"
#include "scrapers/tv_show/omdb/OmdbTvShowSearchJob.h"
#include "settings/Settings.h"
#include "test/helpers/scraper_helpers.h"
#include "test/mocks/settings/SettingsMock.h"

#include "data/tv_show/TvShowEpisode.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

static OmdbApi& getOmdbApi()
{
    static auto api = std::make_unique<OmdbApi>();
    if (!api->isInitialized()) {
        QString apiKey = QString::fromLocal8Bit(qgetenv("MEDIAELCH_OMDB_API_KEY"));
        api->setApiKey(apiKey);
        api->initialize();
    }
    return *api;
}

static bool hasOmdbApiKey()
{
    return !qgetenv("MEDIAELCH_OMDB_API_KEY").isEmpty();
}

TEST_CASE("OmdbTv returns valid search results", "[tv][OMDb][OmdbTv][search]")
{
    if (!hasOmdbApiKey()) {
        WARN("MEDIAELCH_OMDB_API_KEY not set — skipping OMDb TV tests");
        return;
    }

    SECTION("Search by show name returns correct results")
    {
        ShowSearchJob::Config config{"Breaking Bad", mediaelch::Locale::English};
        auto* searchJob = new OmdbTvShowSearchJob(getOmdbApi(), config);
        const auto scraperResults = test::searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() >= 1);
        CHECK(scraperResults[0].title == "Breaking Bad");
    }

    SECTION("Search by IMDB ID returns single correct result")
    {
        ShowSearchJob::Config config{"tt0903747", mediaelch::Locale::English};
        auto* searchJob = new OmdbTvShowSearchJob(getOmdbApi(), config);
        const auto scraperResults = test::searchTvScraperSync(searchJob).first;

        REQUIRE(scraperResults.length() == 1);
        CHECK(scraperResults[0].title == "Breaking Bad");
        CHECK(scraperResults[0].identifier.str() == "tt0903747");
    }
}

TEST_CASE("OmdbTv scrapes correct show details", "[tv][OMDb][OmdbTv][load_data]")
{
    if (!hasOmdbApiKey()) {
        WARN("MEDIAELCH_OMDB_API_KEY not set — skipping OMDb TV tests");
        return;
    }

    SECTION("Show loaded by IMDB ID has expected fields")
    {
        // Breaking Bad
        ShowScrapeJob::Config config{
            ShowIdentifier("tt0903747"), mediaelch::Locale::English, mediaelch::allShowScraperInfos()};
        auto scrapeJob = std::make_unique<OmdbTvShowScrapeJob>(getOmdbApi(), config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        CHECK(show.title() == "Breaking Bad");
        CHECK(show.firstAired().year() == 2008);
        CHECK(show.certification().toString() == "TV-MA");
        CHECK(show.genres().size() >= 1);
        CHECK_THAT(show.overview(), Contains("Walter White"));
        CHECK(show.ratings().hasSource("imdb"));
    }
}

TEST_CASE("OmdbTv scrapes season episodes", "[tv][OMDb][OmdbTv][season]")
{
    if (!hasOmdbApiKey()) {
        WARN("MEDIAELCH_OMDB_API_KEY not set — skipping OMDb TV tests");
        return;
    }

    SECTION("Season 1 of Breaking Bad has correct episode count and titles")
    {
        SeasonScrapeJob::Config config{ShowIdentifier("tt0903747"),
            Locale::English,
            {SeasonNumber(1)},
            SeasonOrder::Aired,
            {EpisodeScraperInfo::Title, EpisodeScraperInfo::FirstAired, EpisodeScraperInfo::Rating}};

        auto scrapeJob = std::make_unique<OmdbTvSeasonScrapeJob>(getOmdbApi(), config);
        test::scrapeSeasonSync(scrapeJob.get());
        const auto& episodes = scrapeJob->episodes();

        REQUIRE(episodes.size() == 7); // Breaking Bad Season 1 has 7 episodes

        TvShowEpisode* pilot = episodes[{SeasonNumber(1), EpisodeNumber(1)}];
        REQUIRE(pilot != nullptr);
        CHECK(pilot->title() == "Pilot");
        CHECK(pilot->firstAired().isValid());
    }
}

TEST_CASE("OmdbTv scrapes single episode details", "[tv][OMDb][OmdbTv][episode]")
{
    if (!hasOmdbApiKey()) {
        WARN("MEDIAELCH_OMDB_API_KEY not set — skipping OMDb TV tests");
        return;
    }

    SECTION("Breaking Bad S01E01 loaded by IMDB ID has expected fields")
    {
        // Breaking Bad - Pilot (S01E01)
        EpisodeIdentifier id("tt0959621");
        EpisodeScrapeJob::Config config{id, Locale::English, mediaelch::allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<OmdbTvEpisodeScrapeJob>(getOmdbApi(), config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        CHECK(episode.title() == "Pilot");
        CHECK(episode.firstAired().year() == 2008);
        CHECK_THAT(episode.overview(), Contains("Walter White"));
        CHECK(episode.ratings().hasSource("imdb"));
    }
}
