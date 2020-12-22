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
    if (!getTmdbApi().isInitialized()) {
        QEventLoop loop;
        QEventLoop::connect(&getTmdbApi(), &TmdbApi::initialized, [&]() { loop.quit(); });
        getTmdbApi().initialize();
        loop.exec();
    }
}
