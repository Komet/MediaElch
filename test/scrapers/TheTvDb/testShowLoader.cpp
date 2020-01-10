#include "test/test_helpers.h"

#include "test/mocks/settings/MockScraperSettings.h"

#include "scrapers/tv_show/TheTvDb/EpisodeLoader.h"
#include "scrapers/tv_show/TheTvDb/ShowLoader.h"

#include "settings/Settings.h"

#include <chrono>

using namespace thetvdb;
using namespace std::chrono_literals;

static void useDvdOrder(bool isDvd)
{
    Settings::instance()->setTvShowDvdOrder(isDvd);
}

static void loadShowSync(ShowLoader& scraper)
{
    QEventLoop loop;
    loop.connect(&scraper, &ShowLoader::sigLoadDone, [&]() { loop.quit(); });
    // Use a timer because loadShowAndEpisodes() may have already finished before the loop
    // is even executed. This can happen if all items are cached.
    QTimer::singleShot(0, [&scraper]() { scraper.loadShowAndEpisodes(); });
    loop.exec();
}

TEST_CASE("TheTvDb ShowLoader scrapes show data", "[scraper][TheTvDb][load_data][requires_internet]")
{
    TvDbId scrubsId(76156);
    useDvdOrder(false);

    SECTION("Show has correct details")
    {
        TvShow t;
        t.setTvdbId(scrubsId);
        ShowLoader showLoader(t, "en", ShowLoader::scraperInfos, EpisodeLoader::scraperInfos, TvShowUpdateType::Show);
        loadShowSync(showLoader);

        REQUIRE(t.id() == scrubsId);
        REQUIRE(t.tvdbId() == scrubsId);

        CHECK(t.name() == "Scrubs");
        CHECK(t.imdbId() == ImdbId("tt0285403"));
        CHECK(t.certification() == Certification("TV-PG"));
        CHECK(t.firstAired() == QDate(2001, 10, 2));
        CHECK(t.network() == "ABC (US)");
        CHECK_THAT(t.overview(), Contains("Scrubs focuses on the lives of several people"));

        REQUIRE_FALSE(t.ratings().isEmpty());
        // Ratings are at the moment always 0
        // CHECK(t.ratings().first().rating == Approx(9).margin(0.5));
        // CHECK(t.ratings().first().voteCount > 290);

        CHECK(t.runtime() == 25min);
        CHECK(t.status() == "Ended");

        const auto& actors = t.actors();
        REQUIRE(actors.size() > 0);
        CHECK(actors[0]->name == "Aloma Wright");

        const auto& genres = t.genres();
        REQUIRE(genres.size() > 0);
        CHECK_THAT(genres[0], Contains("Comedy"));

        CHECK(t.backdrops().size() > 25);
        CHECK(t.posters().size() > 15);
        CHECK(t.seasonPosters(SeasonNumber(1)).size() > 4);
        CHECK(t.seasonBanners(SeasonNumber(1)).size() > 1);
        CHECK(t.banners().size() > 11);
    }
}

TEST_CASE("TheTvDb ShowLoader scrapes episodes", "[scraper][TheTvDb][load_data][requires_internet]")
{
    TvDbId scrubsId("76156");
    useDvdOrder(false);

    const auto setupShow = [](TvShow& show, TvShowEpisode& episode) {
        episode.setShow(&show);
        episode.setSeason(SeasonNumber(1));
        episode.setEpisode(EpisodeNumber(1));
        show.addEpisode(&episode);
    };

    const auto checkFirstEpisodeInfo = [](const char* msg, const TvShowEpisode& e) {
        INFO(msg);

        CHECK(e.name() == "My First Day");
        CHECK_THAT(e.overview(), StartsWith("John Dorian, \"J.D.\" to friends"));
        CHECK(e.imdbId() == ImdbId("tt0696640"));

        REQUIRE_FALSE(e.ratings().isEmpty());
        // Ratings are at the moment always 0
        // CHECK(e.ratings().first().rating == Approx(7.6).margin(0.5));
        // CHECK(e.ratings().first().voteCount >= 80);

        CHECK(e.firstAired() == QDate(2001, 10, 2));
        CHECK_THAT(e.overview(), Contains(""));

        REQUIRE(e.directors().size() > 0);
        CHECK(e.directors()[0] == "Adam Bernstein");

        REQUIRE(e.writers().size() > 0);
        CHECK(e.writers()[0] == "Bill Lawrence");

        REQUIRE(e.actors().size() == 0); // TheTvDb does not have actors for episodes, only guest stars
    };

    SECTION("Show's episodes have correct details")
    {
        // Setup show and episode(s)
        TvShow show;
        show.setTvdbId(scrubsId);
        TvShowEpisode episode;
        setupShow(show, episode);
        show.setName("Name should not be updated");

        ShowLoader showLoader(
            show, "en", ShowLoader::scraperInfos, EpisodeLoader::scraperInfos, TvShowUpdateType::AllEpisodes);
        loadShowSync(showLoader);

        REQUIRE(show.tvdbId() == scrubsId);
        // Show should not be updated, because update type is AllEpisodes
        CHECK(show.id() == scrubsId);
        CHECK(show.name() == "Name should not be updated");

        // Check episode that was loaded but not merged with @episode
        CHECK(episode.name() == "");
        const auto& e = *showLoader.parser().episodes().at(0);
        checkFirstEpisodeInfo("using the parser's first episode", e);
    }

    SECTION("Show's new episodes have correct details after merging")
    {
        // Setup show and episode(s)
        TvShow show;
        show.setTvdbId(scrubsId);
        TvShowEpisode episode;
        setupShow(show, episode);
        show.setName("Name should not be updated");

        ShowLoader showLoader(
            show, "en", ShowLoader::scraperInfos, EpisodeLoader::scraperInfos, TvShowUpdateType::AllEpisodes);
        loadShowSync(showLoader);

        REQUIRE(show.tvdbId() == scrubsId);
        // Show should not be updated, because update type is AllEpisodes
        CHECK(show.id() == scrubsId);
        CHECK(show.name() == "Name should not be updated");

        const auto& e = *showLoader.parser().episodes().at(0);
        checkFirstEpisodeInfo("using the parser's first episode", e);

        showLoader.mergeEpisodesToShow();
        checkFirstEpisodeInfo("using show's first episode", episode);
    }
}


TEST_CASE("TheTvDb ShowLoader respects DVD/Official order", "[scraper][TheTvDb][load_data][requires_internet]")
{
    const auto setupSpace1999 = [](TvShow& show, TvShowEpisode& episode) {
        episode.setShow(&show);
        episode.setSeason(SeasonNumber(1));
        episode.setEpisode(EpisodeNumber(2));
        show.addEpisode(&episode);
    };

    const auto loadSpace1999Season1Episode2 = [&setupSpace1999](bool dvdOrder, const QString& expectedTitle) {
        CAPTURE(dvdOrder);
        useDvdOrder(dvdOrder);

        TvDbId spaceId("76366");
        TvShow show;
        show.setTvdbId(spaceId);
        TvShowEpisode episode;
        setupSpace1999(show, episode);

        ShowLoader showLoader(
            show, "en", {TvShowScraperInfos::Title}, {TvShowScraperInfos::Title}, TvShowUpdateType::NewEpisodes);
        loadShowSync(showLoader);
        showLoader.mergeEpisodesToShow();

        REQUIRE(show.tvdbId() == spaceId);
        CHECK(episode.name() == expectedTitle);
    };

    SECTION("'Space: 1999' S01E02 is loaded in DVD order")
    {
        loadSpace1999Season1Episode2(true, "Matter of Life and Death");
    }

    SECTION("'Space: 1999' S01E02 is loaded in Official order") //
    {
        loadSpace1999Season1Episode2(false, "Force of Life");
    }
}
