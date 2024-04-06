#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/fernsehserien_de/FernsehserienDe.h"

#include "test/helpers/scraper_helpers.h"

using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("FernsehserienDe scrapes episode details", "[episode][FernsehserienDe][load_data]")
{
    auto api = std::make_unique<FernsehserienDeApi>();


    SECTION("Loads all details with episode FernsehserienDe ID")
    {
        // https://www.fernsehserien.de/die-simpsons/folgen/12x19-wunder-gibt-es-immer-wieder-62300
        QString episodeId("die-simpsons/folgen/12x19-wunder-gibt-es-immer-wieder-62300");
        EpisodeIdentifier id(episodeId);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<FernsehserienDeEpisodeScrapeJob>(*api, config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE_FALSE(episode.title().isEmpty());
        test::scraper::compareAgainstReference(episode, "scrapers/fernsehserien_de/The-Simpsons-S12E19-with-id");
    }

    SECTION("Loads all details of episode with thumbnail")
    {
        // https://www.fernsehserien.de/die-simpsons/folgen/31x01-der-winter-unseres-monetarisierten-vergnuegens-1306622
        QString episodeId("die-simpsons/folgen/31x01-der-winter-unseres-monetarisierten-vergnuegens-1306622");
        EpisodeIdentifier id(episodeId);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<FernsehserienDeEpisodeScrapeJob>(*api, config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE_FALSE(episode.title().isEmpty());
        test::scraper::compareAgainstReference(episode, "scrapers/fernsehserien_de/The-Simpsons-S31E01");
    }

    SECTION("Loads details with season and episode number of show with >10 seasons")
    {
        // https://www.fernsehserien.de/die-simpsons/folgen/12x19-wunder-gibt-es-immer-wieder-62300
        QString showId("die-simpsons");
        SeasonNumber season(12);
        EpisodeNumber episodeNumber(19);
        EpisodeIdentifier id(showId, season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<FernsehserienDeEpisodeScrapeJob>(*api, config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE_FALSE(episode.title().isEmpty());
        test::scraper::compareAgainstReference(episode, "scrapers/fernsehserien_de/The-Simpsons-S12E19-no-id");
    }

    SECTION("Loads details with season and episode number of show with <10 seasons")
    {
        // In contrast to >10 seasons above, the season overview page only has one digit for the season number.

        QString showId("black-mirror");
        SeasonNumber season(4);
        EpisodeNumber episodeNumber(5);

        EpisodeIdentifier id(showId, season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<FernsehserienDeEpisodeScrapeJob>(*api, config);
        test::scrapeEpisodeSync(scrapeJob.get());
        auto& episode = scrapeJob->episode();

        REQUIRE_FALSE(episode.title().isEmpty());
        test::scraper::compareAgainstReference(episode, "scrapers/fernsehserien_de/Black-Mirror-S04E05-no-id");
    }

    SECTION("Returns error for non-existent episode via season/episode number")
    {
        // This show has no episodes on fernsehserien.de!
        QString showId("ya-pas-dage");
        SeasonNumber season(4);
        EpisodeNumber episodeNumber(5);

        EpisodeIdentifier id(showId, season, episodeNumber, SeasonOrder::Aired);
        EpisodeScrapeJob::Config config{id, Locale("de-DE"), allEpisodeScraperInfos()};

        auto scrapeJob = std::make_unique<FernsehserienDeEpisodeScrapeJob>(*api, config);
        test::scrapeEpisodeSync(scrapeJob.get(), true);

        CHECK(scrapeJob->episode().title().isEmpty());
        CHECK(scrapeJob->hasError());
    }

    // SECTION("Returns error for non-existent episode via fabricated ID")
    // {
    //     // This show has no episodes on fernsehserien.de!
    //     EpisodeIdentifier id("ya-pas-dage/folgen/4x05-i-do-not-exist-4231");
    //     EpisodeScrapeJob::Config config{id, Locale("de-DE"), allEpisodeScraperInfos()};

    //     auto scrapeJob = std::make_unique<FernsehserienDeEpisodeScrapeJob>(*api, config);
    //     test::scrapeEpisodeSync(scrapeJob.get(), true);

    //     CAPTURE(scrapeJob->errorString());
    //     CAPTURE(scrapeJob->errorText());
    //     CAPTURE(scrapeJob->errorCode());

    //     CHECK(scrapeJob->episode().title().isEmpty());
    //     CHECK(scrapeJob->hasError());
    // }
}
