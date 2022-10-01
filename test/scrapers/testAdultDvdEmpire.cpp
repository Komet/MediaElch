#include "test/test_helpers.h"

#include "scrapers/movie/adultdvdempire/AdultDvdEmpire.h"

#include <chrono>

using namespace std::chrono_literals;
using namespace mediaelch::scraper;

/// @brief Loads movie data synchronously
void loadAdultDvdEmpireSync(AdultDvdEmpire& scraper, QHash<MovieScraper*, MovieIdentifier> ids, Movie& movie)
{
    const auto infos = scraper.meta().supportedDetails;
    loadDataSync(scraper, ids, movie, infos);
}


TEST_CASE("AdultDvdEmpire scrapes correct movie details", "[AdultDvdEmpire][load_data]")
{
    AdultDvdEmpire hm;

    SECTION("Movie has correct details for DVD movie")
    {
        Movie m(QStringList{}); // Movie without files
        loadAdultDvdEmpireSync(hm, {{nullptr, MovieIdentifier("/1745335/magic-mike-xxxl-porn-movies.html")}}, m);

        REQUIRE_THAT(m.name(), StartsWith("Magic Mike XXXL"));
        test::scraper::compareAgainstReference(m, "scrapers/ade/DVD-Magic-Mike-1745335");
    }

    SECTION("Movie has correct details for VOD movie")
    {
        Movie m(QStringList{}); // Movie without files
        loadAdultDvdEmpireSync(hm, {{nullptr, MovieIdentifier("/1670507/50-shades-of-pink-porn-videos.html")}}, m);

        REQUIRE_THAT(m.name(), StartsWith("50 Shades Of Pink"));
        test::scraper::compareAgainstReference(m, "scrapers/ade/VOD-50-Shades-1670507");
    }
}
