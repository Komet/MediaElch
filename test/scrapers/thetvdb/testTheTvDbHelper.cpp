#include "test/scrapers/thetvdb/testTheTvDbHelper.h"

#include "test/test_helpers.h"

#include <QEventLoop>

using namespace mediaelch::scraper;

TheTvDbApi& getTheTvDbApi()
{
    static auto api = std::make_unique<TheTvDbApi>();
    return *api;
}

void waitForTheTvDbInitialized()
{
    if (!getTheTvDbApi().isInitialized()) {
        QEventLoop loop;
        QEventLoop::connect(&getTheTvDbApi(), &TheTvDbApi::initialized, [&]() { loop.quit(); });
        getTheTvDbApi().initialize();
        loop.exec();
    }
}
