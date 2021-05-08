#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

#include <memory>

mediaelch::scraper::UniversalMusicScraper& getUniversalMusicScraper()
{
    static auto api = std::make_unique<mediaelch::scraper::UniversalMusicScraper>();
    return *api;
}
