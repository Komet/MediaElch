#include "test/scrapers/imdbtv/testImdbTvHelper.h"

#include "test/test_helpers.h"

using namespace mediaelch::scraper;

ImdbApi& getImdbApi()
{
    static auto api = std::make_unique<ImdbApi>();
    return *api;
}
