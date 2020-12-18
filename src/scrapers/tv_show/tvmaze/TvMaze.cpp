#include "scrapers/tv_show/tvmaze/TvMaze.h"

#include "scrapers/tv_show/tvmaze/TvMazeEpisodeScrapeJob.h"
#include "scrapers/tv_show/tvmaze/TvMazeSeasonScrapeJob.h"
#include "scrapers/tv_show/tvmaze/TvMazeShowScrapeJob.h"
#include "scrapers/tv_show/tvmaze/TvMazeShowSearchJob.h"
#include "tv_shows/TvShow.h"

namespace mediaelch {
namespace scraper {

QString TvMaze::ID = "tvmaze";

TvMaze::TvMaze(QObject* parent) : TvScraper(parent)
{
    m_meta.identifier = TvMaze::ID;
    m_meta.name = "TVmaze";
    m_meta.description = tr("TVmaze is a collaborative site, which can be edited by any registered user. "
                            "Find episode information for any show on any device. anytime, anywhere!");
    m_meta.website = "https://www.tvmaze.com";
    m_meta.termsOfService = "https://www.tvmaze.com/site/tos";
    m_meta.privacyPolicy = "https://www.tvmaze.com/site/privacy";
    m_meta.help = "https://www.tvmaze.com/forums";
    m_meta.supportedShowDetails = {
        ShowScraperInfo::Title,
        ShowScraperInfo::Poster,
        ShowScraperInfo::Fanart,
        ShowScraperInfo::SeasonPoster,
        ShowScraperInfo::Banner,
        ShowScraperInfo::Network,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::Status,
        ShowScraperInfo::Actors,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Rating,
        ShowScraperInfo::ExtraArts //
    };
    m_meta.supportedEpisodeDetails = {
        EpisodeScraperInfo::Title,
        EpisodeScraperInfo::FirstAired,
        EpisodeScraperInfo::Thumbnail,
        EpisodeScraperInfo::Overview //
    };
    m_meta.supportedSeasonOrders = {SeasonOrder::Aired};
    m_meta.supportedLanguages = {Locale::English};
    m_meta.defaultLocale = Locale::English;
}

const TvScraper::ScraperMeta& TvMaze::meta() const
{
    return m_meta;
}

void TvMaze::initialize()
{
    // no-op: TvMaze does not require any keys or tokens.
}

bool TvMaze::isInitialized() const
{
    return true;
}

ShowSearchJob* TvMaze::search(ShowSearchJob::Config config)
{
    qInfo() << "[TvMaze] Search for:" << config.query;
    auto* searchJob = new TvMazeShowSearchJob(m_api, std::move(config));
    return searchJob;
}

ShowScrapeJob* TvMaze::loadShow(ShowScrapeJob::Config config)
{
    qInfo() << "[TvMaze] Load TV show with id:" << config.identifier;
    auto* loader = new TvMazeShowScrapeJob(m_api, config, this);
    return loader;
}

SeasonScrapeJob* TvMaze::loadSeasons(SeasonScrapeJob::Config config)
{
    qInfo() << "[TvMaze] Load season with show id:" << config.showIdentifier;
    auto* loader = new TvMazeSeasonScrapeJob(m_api, config, this);
    return loader;
}

EpisodeScrapeJob* TvMaze::loadEpisode(EpisodeScrapeJob::Config config)
{
    qDebug() << "[TvMaze] Load single episode of TV show with id:" << config.identifier;
    auto* loader = new TvMazeEpisodeScrapeJob(m_api, config, this);
    return loader;
}


} // namespace scraper
} // namespace mediaelch
