#include "test/scrapers/imdbtv/testImdbTvHelper.h"

#include "test/test_helpers.h"

using namespace mediaelch::scraper;

ImdbTvApi& getImdbTvApi()
{
    static auto api = std::make_unique<ImdbTvApi>();
    return *api;
}
