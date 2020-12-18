#include "test/scrapers/tvmaze/testTvMazeHelper.h"

#include "test/test_helpers.h"

using namespace mediaelch::scraper;

TvMazeApi& getTvMazeApi()
{
    static auto api = std::make_unique<TvMazeApi>();
    return *api;
}
