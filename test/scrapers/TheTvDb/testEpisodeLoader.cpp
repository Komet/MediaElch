#include "test/test_helpers.h"

#include "test/mocks/settings/MockScraperSettings.h"

#include "scrapers/tv_show/TheTvDb/EpisodeLoader.h"

using namespace thetvdb;

/**
 * @brief Loads movie data synchronously
 */
static void loadEpisodeSync(EpisodeLoader& scraper)
{
    QEventLoop loop;
    loop.connect(&scraper, &EpisodeLoader::sigLoadDone, [&]() { loop.quit(); });
    scraper.loadData();
    loop.exec();
}

TEST_CASE("TheTvDb EpisodeLoader scrapes single episode", "[scraper][TheTvDb][episode][load_data][requires_internet]")
{
    TvShowEpisode e; // is set in each individual section
    EpisodeLoader episodeLoader(TvDbId("76156"), e, "en", EpisodeLoader::scraperInfos);

    SECTION("Episode with TheTvDb id has correct details")
    {
        e.setTvdbId(TvDbId("184607"));
        loadEpisodeSync(episodeLoader);

        REQUIRE(e.tvdbId() == TvDbId("184607"));
        CHECK(e.imdbId() == ImdbId("tt0696544"));
        CHECK(e.episode() == EpisodeNumber(6));
        CHECK(e.season() == SeasonNumber(1));
        CHECK(e.name() == "My Bad");

        CHECK(e.firstAired() == QDate(2001, 10, 30));
        CHECK_THAT(e.overview(), StartsWith("Dr. Cox is still facing the threat of suspension"));
        REQUIRE_FALSE(e.ratings().isEmpty());
        // Ratings are at the moment always 0
        // CHECK(e.ratings().first().rating == Approx(7).margin(0.5));

        REQUIRE(e.directors().size() > 0);
        CHECK(e.directors()[0] == "Marc Buckland");

        REQUIRE(e.writers().size() > 0);
        CHECK(e.writers()[0] == "Gabrielle Allan");

        // todo: thumbnail?
    }

    SECTION("Episode without TheTvDb id on first api page has correct details")
    {
        e.setTvdbId(TvDbId(""));
        e.setSeason(SeasonNumber(7));
        e.setEpisode(EpisodeNumber(10));
        loadEpisodeSync(episodeLoader);

        REQUIRE(e.tvdbId() == TvDbId("359981"));
        CHECK(e.imdbId() == ImdbId("tt1031512"));
        CHECK(e.name() == "My Waste of Time");

        CHECK(e.firstAired() == QDate(2008, 5, 1));
        CHECK_THAT(e.overview(), Contains("Dr. Cox contemplates his new job title"));
        REQUIRE_FALSE(e.ratings().isEmpty());
        // Ratings are at the moment always 0
        // CHECK(e.ratings().first().rating == Approx(7.6).margin(0.5));

        REQUIRE(e.directors().size() > 0);
        CHECK(e.directors()[0] == "Chris Koch");

        REQUIRE(e.writers().size() > 0);
        CHECK(e.writers()[0] == "Andy Schwartz");
    }

    SECTION("Episode without TheTvDb id on last api page has correct details")
    {
        // Yu-Gi-Oh! has season with more than 30 episodes (-> page size)
        TvShowEpisode y;
        EpisodeLoader episodeLoaderYuGiOh(TvDbId("76894"), y, "en", EpisodeLoader::scraperInfos);

        y.setTvdbId(TvDbId(""));
        y.setSeason(SeasonNumber(1));
        y.setEpisode(EpisodeNumber(49));

        loadEpisodeSync(episodeLoaderYuGiOh);

        REQUIRE(y.tvdbId() == TvDbId("219339"));
        CHECK(y.name() == "Dungeon Dice Monsters (Part 4 of 4)");
        CHECK(y.firstAired() == QDate(2002, 11, 9));
    }
}
