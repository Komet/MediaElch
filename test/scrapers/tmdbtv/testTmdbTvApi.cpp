#include "test/test_helpers.h"

#include "scrapers/tv_show/tmdb/TmdbTvApi.h"

using namespace mediaelch::scraper;

TEST_CASE("TmdbTvApi loads configuration", "[show][TmdbTv][load_data]")
{
    TmdbTvApi api;

    QEventLoop loop;
    QEventLoop::connect(&api, &TmdbTvApi::initialized, [&]() { loop.quit(); });
    api.initialize();
    loop.exec();

    // These normally do not change. We still load them from TMDb.
    CHECK(api.config().imageBaseUrl == "http://image.tmdb.org/t/p/");
    CHECK(api.config().imageSecureBaseUrl == "https://image.tmdb.org/t/p/");
    CHECK_THAT(api.config().backdropSizes, Contains("original"));
    CHECK_THAT(api.config().logoSizes, Contains("original"));
    CHECK_THAT(api.config().posterSizes, Contains("original"));
    CHECK_THAT(api.config().profileSizes, Contains("original"));
    CHECK_THAT(api.config().stillSizes, Contains("original"));
}
