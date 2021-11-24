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
    TheTvDbApi& api = getTheTvDbApi();
    if (!api.isInitialized()) {
        QEventLoop loop;
        QEventLoop::connect(&api, &TheTvDbApi::initialized, &loop, &QEventLoop::quit);
        api.initialize();
        loop.exec();
    }
}
