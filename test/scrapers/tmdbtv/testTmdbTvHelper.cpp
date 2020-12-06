#include "test/scrapers/tmdbtv/testTmdbTvHelper.h"

#include "test/test_helpers.h"

using namespace mediaelch::scraper;

TmdbTvApi& getTmdbTvApi()
{
    static auto api = std::make_unique<TmdbTvApi>();
    return *api;
}

void waitForTmdbTvInitialized()
{
    if (!getTmdbTvApi().isInitialized()) {
        QEventLoop loop;
        QEventLoop::connect(&getTmdbTvApi(), &TmdbTvApi::initialized, [&]() { loop.quit(); });
        getTmdbTvApi().initialize();
        loop.exec();
    }
}
