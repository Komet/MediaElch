#include "scrapers/tv_show/custom/CustomTvScraperConfiguration.h"

#include "scrapers/tv_show/custom/CustomTvScraper.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"

namespace mediaelch {
namespace scraper {

CustomTvScraperConfiguration::CustomTvScraperConfiguration(Settings& settings,
    TmdbTv& _tmdbTv,
    ImdbTv& _imdbTv,
    CustomTvScraperConfiguration::ScraperForShowDetails _scraperForShowDetails,
    CustomTvScraperConfiguration::ScraperForEpisodeDetails _scraperForEpisodeDetails) :
    ScraperConfiguration(QString(CustomTvScraper::ID), settings),
    m_settings{settings},
    tmdbTv{&_tmdbTv},
    imdbTv{&_imdbTv},
    scraperForShowDetails{std::move(_scraperForShowDetails)},
    scraperForEpisodeDetails{std::move(_scraperForEpisodeDetails)}
{
    Q_UNUSED(m_settings);
}

TvScraper* CustomTvScraperConfiguration::scraperForId(const QString& id) const
{
    if (id == TmdbTv::ID) {
        return tmdbTv;
    }
    if (id == ImdbTv::ID) {
        return imdbTv;
    }
    return nullptr;
}

void CustomTvScraperConfiguration::init()
{
    // TODO
}

} // namespace scraper
} // namespace mediaelch
