#include "scrapers/tv_show/thetvdb/TheTvDb.h"

#include "scrapers/tv_show/thetvdb/TheTvDbEpisodeScrapeJob.h"
#include "scrapers/tv_show/thetvdb/TheTvDbSeasonScrapeJob.h"
#include "scrapers/tv_show/thetvdb/TheTvDbShowScrapeJob.h"
#include "scrapers/tv_show/thetvdb/TheTvDbShowSearchJob.h"
#include "tv_shows/TvShow.h"

namespace mediaelch {
namespace scraper {

QString TheTvDb::ID = "thetvdb";

TheTvDb::TheTvDb(QObject* parent) : TvScraper(parent)
{
    m_meta.identifier = TheTvDb::ID;
    m_meta.name = "TheTvDb";
    m_meta.description = tr("TheTvDb is one of the most accurate sources for TV and film. "
                            "Their information comes from fans like you, so create a free account on their website and "
                            "help your favorite shows and movies. "
                            "Everything added is shared not only with MediaElch but many other sites, mobile apps, "
                            "and devices as well.");
    m_meta.website = "https://thetvdb.com";
    m_meta.termsOfService = "https://thetvdb.com/tos";
    m_meta.privacyPolicy = "https://thetvdb.com/privacy-policy";
    m_meta.help = "https://forums.thetvdb.com/";
    m_meta.supportedShowDetails = {ShowScraperInfo::Actors,
        ShowScraperInfo::Banner,
        ShowScraperInfo::Certification,
        ShowScraperInfo::Fanart,
        ShowScraperInfo::FirstAired,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Network,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Poster,
        ShowScraperInfo::Rating,
        ShowScraperInfo::Title,
        ShowScraperInfo::Tags,
        ShowScraperInfo::ExtraArts,
        ShowScraperInfo::SeasonPoster,
        ShowScraperInfo::SeasonBanner,
        ShowScraperInfo::ExtraFanarts,
        ShowScraperInfo::Thumb,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::Status};
    m_meta.supportedEpisodeDetails = {EpisodeScraperInfo::Actors,
        EpisodeScraperInfo::Certification,
        EpisodeScraperInfo::Director,
        EpisodeScraperInfo::FirstAired,
        EpisodeScraperInfo::Overview,
        EpisodeScraperInfo::Rating,
        EpisodeScraperInfo::Thumbnail,
        EpisodeScraperInfo::Title,
        EpisodeScraperInfo::Writer};
    m_meta.supportedSeasonOrders = {SeasonOrder::Aired, SeasonOrder::Dvd};
    m_meta.supportedLanguages = {"bg",
        "zh",
        "hr",
        "cs",
        "da",
        "nl",
        "en",
        "fi",
        "fr",
        "de",
        "el",
        "he",
        "hu",
        "it",
        "ja",
        "ko",
        "no",
        "pl",
        "pt",
        "ru",
        "sl",
        "es",
        "sv",
        "tr"};
    m_meta.defaultLocale = Locale("en");

    connect(&m_api, &TheTvDbApi::initialized, this, [this](bool wasSuccessful) {
        m_isInitialized = wasSuccessful;
        emit initialized(wasSuccessful, this);
    });
}

const TvScraper::ScraperMeta& TheTvDb::meta() const
{
    return m_meta;
}

void TheTvDb::initialize()
{
    m_api.initialize();
}

bool TheTvDb::isInitialized() const
{
    return m_isInitialized;
}

ShowSearchJob* TheTvDb::search(ShowSearchJob::Config config)
{
    qInfo() << "[TheTvDb] Search for:" << config.query;
    auto* searchJob = new TheTvDbShowSearchJob(m_api, std::move(config));
    return searchJob;
}

ShowScrapeJob* TheTvDb::loadShow(ShowScrapeJob::Config config)
{
    qInfo() << "[TheTvDb] Load TV show with id:" << config.identifier;
    auto* loader = new TheTvDbShowScrapeJob(m_api, config, this);
    return loader;
}

SeasonScrapeJob* TheTvDb::loadSeasons(SeasonScrapeJob::Config config)
{
    qInfo() << "[TheTvDb] Load season with show id:" << config.showIdentifier;
    auto* loader = new TheTvDbSeasonScrapeJob(m_api, config, this);
    return loader;
}

EpisodeScrapeJob* TheTvDb::loadEpisode(EpisodeScrapeJob::Config config)
{
    qDebug() << "[TheTvDb] Load single episode of TV show with id:" << config.identifier;
    auto* loader = new TheTvDbEpisodeScrapeJob(m_api, config, this);
    return loader;
}


} // namespace scraper
} // namespace mediaelch
