#include "scrapers/tv_show/custom/CustomTvScraper.h"

#include "scrapers/tv_show/custom/CustomEpisodeScrapeJob.h"
#include "scrapers/tv_show/custom/CustomSeasonScrapeJob.h"
#include "scrapers/tv_show/custom/CustomShowScrapeJob.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowSearchJob.h"
#include "settings/Settings.h"

#include <QSet>
#include <QTimer>
#include <algorithm>

namespace mediaelch {
namespace scraper {

QString CustomTvScraper::ID = "customtvscraper";

QVector<QString> CustomTvScraper::supportedScraperIds()
{
    return {TmdbTv::ID, TheTvDb::ID, ImdbTv::ID};
}

CustomTvScraper::CustomTvScraper(CustomTvScraperConfig config, QObject* parent) :
    TvScraper(parent), m_customConfig{std::move(config)}
{
    m_meta.name = tr("Custom TV scraper");
    m_meta.identifier = "custom_tv_scraper";
    m_meta.supportedLanguages = {Locale::NoLocale};
    m_meta.defaultLocale = Locale::NoLocale;
    m_meta.description =
        tr("The custom TV scraper combines multiple scrapers so that details can be "
           "loaded from different sites in one step. It depends on TMDb for loading other scraper IDs.");
    m_meta.help = QUrl("https://mediaelch.github.io/mediaelch-doc/tvshow/index.html");
    // TODO: Union of all used scrapers.
    m_meta.supportedShowDetails = allShowScraperInfos();
    m_meta.supportedEpisodeDetails = allEpisodeScraperInfos();
    m_meta.supportedSeasonOrders = {SeasonOrder::Aired};
}

const TvScraper::ScraperMeta& CustomTvScraper::meta() const
{
    return m_meta;
}

void CustomTvScraper::initialize()
{
    // Simply (re-)initialize the used scrapers.
    m_customConfig.imdbTv->initialize();
    m_customConfig.tmdbTv->initialize();
    m_customConfig.theTvDb->initialize();
}

bool CustomTvScraper::isInitialized() const
{
    const QVector<TvScraper*> scrapers = supportedScrapers();
    return std::all_of(scrapers.cbegin(), scrapers.cend(), [](TvScraper* scraper) { return scraper->isInitialized(); });
}

ShowSearchJob* CustomTvScraper::search(ShowSearchJob::Config config)
{
    // Search is hard coded to TMDb TV because they support multiple IDs for IMDb, TheTbDb, etc.
    return m_customConfig.tmdbTv->search(config);
}

ShowScrapeJob* CustomTvScraper::loadShow(ShowScrapeJob::Config config)
{
    updateScraperDetails(config.details);
    return new CustomShowScrapeJob(m_customConfig, config, this);
}

SeasonScrapeJob* CustomTvScraper::loadSeasons(SeasonScrapeJob::Config config)
{
    updateScraperDetails(config.details);
    return new CustomSeasonScrapeJob(m_customConfig, config, this);
}

EpisodeScrapeJob* CustomTvScraper::loadEpisode(EpisodeScrapeJob::Config config)
{
    updateScraperDetails(config.details);
    return new CustomEpisodeScrapeJob(m_customConfig, config, this);
}

QVector<TvScraper*> CustomTvScraper::supportedScrapers() const
{
    return {m_customConfig.tmdbTv, m_customConfig.theTvDb, m_customConfig.imdbTv};
}

void CustomTvScraper::updateScraperDetails(const QSet<ShowScraperInfo>& details)
{
    // Use current settings.
    m_customConfig.scraperForShowDetails = Settings::instance()->customTvScraperShow();

    // scraperForShowDetails may contain fewer details than were requested.
    // "Normalize" the set by inserting a default scraper for the detail.
    for (ShowScraperInfo info : details) {
        if (!m_customConfig.scraperForShowDetails.contains(info)) {
            m_customConfig.scraperForShowDetails.insert(info, TmdbTv::ID);
        }
    }
}

void CustomTvScraper::updateScraperDetails(const QSet<EpisodeScraperInfo>& details)
{
    // Use current settings.
    m_customConfig.scraperForEpisodeDetails = Settings::instance()->customTvScraperEpisode();

    // scraperForShowDetails may contain fewer details than were requested.
    // "Normalize" the set by inserting a default scraper for the detail.
    for (EpisodeScraperInfo info : details) {
        if (!m_customConfig.scraperForEpisodeDetails.contains(info)) {
            m_customConfig.scraperForEpisodeDetails.insert(info, TmdbTv::ID);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
