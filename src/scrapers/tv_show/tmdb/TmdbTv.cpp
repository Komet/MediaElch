#include "scrapers/tv_show/tmdb/TmdbTv.h"

#include "log/Log.h"
#include "scrapers/tv_show/tmdb/TmdbTvEpisodeScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvSeasonScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"
#include "scrapers/tv_show/tmdb/TmdbTvShowSearchJob.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

QString TmdbTv::ID = "tmdbtv";

TmdbTv::TmdbTv(TmdbTvConfiguration& settings, QObject* parent) : TvScraper(parent), m_settings{settings}
{
    m_meta.identifier = TmdbTv::ID;
    m_meta.name = "TMDB TV";
    m_meta.description = tr("The Movie Database (TMDB) is a community built movie and TV database. "
                            "Every piece of data has been added by our amazing community dating back to 2008. "
                            "TMDB's strong international focus and breadth of data is largely unmatched and "
                            "something we're incredibly proud of. Put simply, we live and breathe community "
                            "and that's precisely what makes us different.");
    m_meta.website = "https://www.themoviedb.org/tv";
    m_meta.termsOfService = "https://www.themoviedb.org/terms-of-use";
    m_meta.privacyPolicy = "https://www.themoviedb.org/privacy-policy";
    m_meta.help = "https://www.themoviedb.org/talk";
    m_meta.supportedShowDetails = {//
        ShowScraperInfo::Title,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Certification,
        ShowScraperInfo::FirstAired,
        ShowScraperInfo::Status,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::Rating,
        ShowScraperInfo::Tags,
        ShowScraperInfo::Poster,
        ShowScraperInfo::Fanart,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Actors,
        ShowScraperInfo::Network,
        ShowScraperInfo::SeasonPoster};
    m_meta.supportedEpisodeDetails = {
        EpisodeScraperInfo::Actors,
        // EpisodeScraperInfo::Certification,
        EpisodeScraperInfo::Director,
        EpisodeScraperInfo::FirstAired,
        // EpisodeScraperInfo::Network,
        EpisodeScraperInfo::Overview,
        EpisodeScraperInfo::Rating,
        EpisodeScraperInfo::Thumbnail,
        EpisodeScraperInfo::Title,
        EpisodeScraperInfo::Writer, //
    };
    m_meta.supportedSeasonOrders = {SeasonOrder::Aired};
    m_meta.supportedLanguages = TmdbTvConfiguration::supportedLanguages();
    m_meta.defaultLocale = TmdbTvConfiguration::defaultLocale();

    connect(&m_api, &TmdbApi::initialized, this, [this](bool wasSuccessful) { emit initialized(wasSuccessful, this); });
}

const TvScraper::ScraperMeta& TmdbTv::meta() const
{
    return m_meta;
}

void TmdbTv::initialize()
{
    m_api.initialize();
}

bool TmdbTv::isInitialized() const
{
    return m_api.isInitialized();
}

ShowSearchJob* TmdbTv::search(ShowSearchJob::Config config)
{
    qCInfo(generic) << "[TmdbTv] Search for:" << config.query;
    return new TmdbTvShowSearchJob(m_api, config, this);
}

ShowScrapeJob* TmdbTv::loadShow(ShowScrapeJob::Config config)
{
    qCInfo(generic) << "[TmdbTv] Load TV show with id:" << config.identifier;
    return new TmdbTvShowScrapeJob(m_api, config, this);
}

SeasonScrapeJob* TmdbTv::loadSeasons(SeasonScrapeJob::Config config)
{
    qCInfo(generic) << "[TmdbTv] Load season with show id:" << config.showIdentifier;
    return new TmdbTvSeasonScrapeJob(m_api, config, this);
}

EpisodeScrapeJob* TmdbTv::loadEpisode(EpisodeScrapeJob::Config config)
{
    qCDebug(generic) << "[TmdbTv] Load single episode of TV show with id:" << config.identifier;
    return new TmdbTvEpisodeScrapeJob(m_api, config, this);
}

} // namespace scraper
} // namespace mediaelch
