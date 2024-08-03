#include "test/test_helpers.h"

#include "data/tv_show/TvShow.h"
#include "scrapers/tv_show/fernsehserien_de/FernsehserienDe.h"

#include "test/helpers/scraper_helpers.h"
#include "test/mocks/settings/SettingsMock.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch;
using namespace mediaelch::scraper;

TEST_CASE("FernsehserienDe scrapes show details", "[show][FernsehserienDe][load_data]")
{
    auto api = std::make_unique<FernsehserienDeApi>();

    // Utility function to load data; since only German is supported, we can easily simplify scraping.
    auto loadAndCompareTvShow = [&api](const QString& id, const QString& filename) {
        SettingsMock settingsMock;
        FernsehserienDeConfiguration scraperConfig(settingsMock);
        FernsehserienDe scraper(scraperConfig);

        ShowScrapeJob::Config config{ShowIdentifier(id), Locale("de-DE"), scraper.meta().supportedShowDetails};

        auto scrapeJob = std::make_unique<FernsehserienDeShowScrapeJob>(*api, config);
        test::scrapeTvScraperSync(scrapeJob.get());
        auto& show = scrapeJob->tvShow();

        REQUIRE_FALSE(show.title().isEmpty());
        test::scraper::compareAgainstReference(show, QStringLiteral("scrapers/fernsehserien_de/%1").arg(filename));
    };

    SECTION("Loads all details for The Simpsons (many episodes)")
    {
        // The Simpsons: A common series with _many_ episodes/seasons.
        // https://www.fernsehserien.de/die-simpsons
        loadAndCompareTvShow("die-simpsons", "Die-Simpsons");
    }

    SECTION("Loads all details for Scrubs")
    {
        // Scrubs has a German-style name: "Scrubs - Die Anfänger" (note the last part).
        // https://www.fernsehserien.de/scrubs
        loadAndCompareTvShow("scrubs", "Scrubs");
    }

    SECTION("Loads all details for Game of Thrones (Genre contains HTML entities)")
    {
        // GoT has a genre "Fantasy- & Sci-Fi-Serien", which contains an HTML "&amp;".
        // https://www.fernsehserien.de/game-of-thrones
        loadAndCompareTvShow("game-of-thrones", "Game-of-Thrones");
    }

    SECTION("Loads all details for non-ASCII character show")
    {
        // Some french TV show; I've never heard of it, but it has a few non-ASCII characters.
        // Note: This show also has no episodes on fernsehserien.de!
        // https://www.fernsehserien.de/ya-pas-dage
        loadAndCompareTvShow("ya-pas-dage", "Y_a-pas-d_age");
    }
}
