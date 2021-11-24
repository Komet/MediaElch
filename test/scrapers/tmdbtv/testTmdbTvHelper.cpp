#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"

#include "test/test_helpers.h"

using namespace mediaelch::scraper;

TmdbApi& getTmdbApi()
{
    static auto api = std::make_unique<TmdbApi>();
    return *api;
}

void waitForTmdbTvInitialized()
{
    TmdbApi& api = getTmdbApi();
    if (!api.isInitialized()) {
        QEventLoop loop;
        QEventLoop::connect(&api, &TmdbApi::initialized, &loop, &QEventLoop::quit);
        api.initialize();
        loop.exec();
    }
}
