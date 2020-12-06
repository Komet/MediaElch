#include "scrapers/tv_show/imdb/ImdbTv.h"

#include "scrapers/tv_show/imdb/ImdbTvEpisodeScrapeJob.h"
#include "scrapers/tv_show/imdb/ImdbTvSeasonScrapeJob.h"
#include "scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h"
#include "scrapers/tv_show/imdb/ImdbTvShowSearchJob.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

QString ImdbTv::ID = "imdbtv";

ImdbTv::ImdbTv(QObject* parent) : TvScraper(parent)
{
    m_meta.identifier = ImdbTv::ID;
    m_meta.name = "IMDb TV";
    m_meta.description = tr("IMDb is the world's most popular and authoritative source for movie, TV "
                            "and celebrity content, designed to help fans explore the world of movies "
                            "and shows and decide what to watch.");
    m_meta.website = "https://www.imdb.com/whats-on-tv/";
    m_meta.termsOfService = "https://www.imdb.com/conditions";
    m_meta.privacyPolicy = "https://www.imdb.com/privacy";
    m_meta.help = "https://help.imdb.com";
    m_meta.supportedShowDetails = {ShowScraperInfo::Title,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Certification,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Rating,
        ShowScraperInfo::Tags,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::FirstAired,
        ShowScraperInfo::Poster};
    m_meta.supportedEpisodeDetails = {EpisodeScraperInfo::Title,
        // EpisodeScraperInfo::Actors,
        EpisodeScraperInfo::Overview,
        EpisodeScraperInfo::Director,
        EpisodeScraperInfo::Writer,
        EpisodeScraperInfo::Thumbnail,
        // EpisodeScraperInfo::Network,
        EpisodeScraperInfo::FirstAired,
        EpisodeScraperInfo::Certification,
        EpisodeScraperInfo::Rating};

    m_meta.supportedSeasonOrders = {SeasonOrder::Aired};
    // The returned language is always based on the user's IP address.
    m_meta.supportedLanguages = {Locale::NoLocale};
    m_meta.defaultLocale = Locale::NoLocale;
}

const TvScraper::ScraperMeta& ImdbTv::meta() const
{
    return m_meta;
}

void ImdbTv::initialize()
{
    QTimer::singleShot(0, [this]() { emit initialized(true, this); });
}

bool ImdbTv::isInitialized() const
{
    return true; // IMDb does not require any setup.
}

ShowSearchJob* ImdbTv::search(ShowSearchJob::Config config)
{
    qInfo() << "[ImdbTv] Search for:" << config.query;
    return new ImdbTvShowSearchJob(m_api, config, this);
}

ShowScrapeJob* ImdbTv::loadShow(ShowScrapeJob::Config config)
{
    qInfo() << "[ImdbTv] Load TV show with id:" << config.identifier;
    return new ImdbTvShowScrapeJob(m_api, config, this);
}

SeasonScrapeJob* ImdbTv::loadSeasons(SeasonScrapeJob::Config config)
{
    qInfo() << "[ImdbTv] Load season with show id:" << config.showIdentifier;
    return new ImdbTvSeasonScrapeJob(m_api, config, this);
}

EpisodeScrapeJob* ImdbTv::loadEpisode(EpisodeScrapeJob::Config config)
{
    qDebug() << "[ImdbTv] Load single episode of TV show with id:" << config.identifier;
    return new ImdbTvEpisodeScrapeJob(m_api, config, this);
}

} // namespace scraper
} // namespace mediaelch
