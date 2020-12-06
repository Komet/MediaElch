#include "scrapers/tv_show/custom/CustomTvScraperConfig.h"

#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"

namespace mediaelch {
namespace scraper {

CustomTvScraperConfig::CustomTvScraperConfig(TmdbTv& _tmdbTv,
    TheTvDb& _theTvDb,
    ImdbTv& _imdbTv,
    CustomTvScraperConfig::ScraperForShowDetails _scraperForShowDetails,
    CustomTvScraperConfig::ScraperForEpisodeDetails _scraperForEpisodeDetails) :
    tmdbTv{&_tmdbTv},
    theTvDb{&_theTvDb},
    imdbTv{&_imdbTv},
    scraperForShowDetails{std::move(_scraperForShowDetails)},
    scraperForEpisodeDetails{std::move(_scraperForEpisodeDetails)}
{
}

TvScraper* CustomTvScraperConfig::scraperForId(const QString& id) const
{
    if (id == TmdbTv::ID) {
        return tmdbTv;
    }
    if (id == ImdbTv::ID) {
        return imdbTv;
    }
    if (id == TheTvDb::ID) {
        return theTvDb;
    }
    return nullptr;
}

// no-op
} // namespace scraper
} // namespace mediaelch
