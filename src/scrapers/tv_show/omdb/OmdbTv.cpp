#include "scrapers/tv_show/omdb/OmdbTv.h"

#include "scrapers/tv_show/omdb/OmdbTvConfiguration.h"
#include "scrapers/tv_show/omdb/OmdbTvEpisodeScrapeJob.h"
#include "scrapers/tv_show/omdb/OmdbTvSeasonScrapeJob.h"
#include "scrapers/tv_show/omdb/OmdbTvShowScrapeJob.h"
#include "scrapers/tv_show/omdb/OmdbTvShowSearchJob.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

OmdbTv::OmdbTv(OmdbTvConfiguration& settings, QObject* parent) : TvScraper(parent), m_settings{settings}
{
    m_meta.identifier = ID;
    m_meta.name = "OMDb TV";
    m_meta.description = tr("The Open Movie Database (OMDb) is a free web service to obtain movie and TV information. "
                            "It provides ratings from IMDB, Rotten Tomatoes, and Metacritic in a single API call. "
                            "Requires a free personal API key from omdbapi.com (free: 1,000 requests/day).");
    m_meta.website = "https://www.omdbapi.com/";
    m_meta.termsOfService = "https://www.omdbapi.com/legal.htm";
    m_meta.privacyPolicy = "https://www.omdbapi.com/legal.htm";
    m_meta.help = "https://www.omdbapi.com/";
    m_meta.supportedShowDetails = {
        ShowScraperInfo::Title,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Certification,
        ShowScraperInfo::FirstAired,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::Rating,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Actors,
        ShowScraperInfo::Poster};
    m_meta.supportedEpisodeDetails = {
        EpisodeScraperInfo::Title,
        EpisodeScraperInfo::Overview,
        EpisodeScraperInfo::FirstAired,
        EpisodeScraperInfo::Director,
        EpisodeScraperInfo::Writer,
        EpisodeScraperInfo::Rating};
    m_meta.supportedSeasonOrders = {SeasonOrder::Aired};
    m_meta.supportedLanguages = {Locale::English};
    m_meta.defaultLocale = Locale::English;

    m_api.setApiKey(m_settings.apiKey());
    connect(&m_settings, &OmdbTvConfiguration::apiKeyChanged, this, [this](const QString& apiKey) {
        m_api.setApiKey(apiKey);
    });

    // OMDb requires no async initialization
    QTimer::singleShot(0, this, [this]() { emit initialized(true, this); });
}

const TvScraper::ScraperMeta& OmdbTv::meta() const
{
    return m_meta;
}

void OmdbTv::initialize()
{
    m_api.initialize();
}

bool OmdbTv::isInitialized() const
{
    return m_api.isInitialized();
}

ShowSearchJob* OmdbTv::search(ShowSearchJob::Config config)
{
    return new OmdbTvShowSearchJob(m_api, config, this);
}

ShowScrapeJob* OmdbTv::loadShow(ShowScrapeJob::Config config)
{
    return new OmdbTvShowScrapeJob(m_api, config, this);
}

SeasonScrapeJob* OmdbTv::loadSeasons(SeasonScrapeJob::Config config)
{
    return new OmdbTvSeasonScrapeJob(m_api, config, this);
}

EpisodeScrapeJob* OmdbTv::loadEpisode(EpisodeScrapeJob::Config config)
{
    return new OmdbTvEpisodeScrapeJob(m_api, config, this);
}

} // namespace scraper
} // namespace mediaelch
