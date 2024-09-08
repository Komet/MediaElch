#include "test/scrapers/universalmusicscraper/testUniversalMusicScraperHelper.h"

#include "scrapers/music/UniversalMusicConfiguration.h"

#include <memory>

mediaelch::scraper::UniversalMusicScraper& getUniversalMusicScraper()
{
    static auto config = std::make_unique<mediaelch::scraper::UniversalMusicConfiguration>(*Settings::instance());
    static auto api = std::make_unique<mediaelch::scraper::UniversalMusicScraper>(*config, nullptr);
    return *api;
}
