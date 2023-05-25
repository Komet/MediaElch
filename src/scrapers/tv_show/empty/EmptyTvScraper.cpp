#include "scrapers/tv_show/empty/EmptyTvScraper.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

QString EmptyTv::ID = "empty";

EmptyTv::EmptyTv(QObject* parent) : TvScraper(parent)
{
    m_meta.identifier = EmptyTv::ID;
    m_meta.name = "Empty TV";
    m_meta.description = "placeholder";
    m_meta.website = "https://www.example.com";
    m_meta.termsOfService = "https://www.example.com/terms";
    m_meta.privacyPolicy = "https://www.example.com/privacy";
    m_meta.help = "https://help.example.com";
    // or e.g. mediaelch::allShowScraperInfos();
    m_meta.supportedShowDetails = {ShowScraperInfo::Title};
    // or e.g. mediaelch::allEpisodeScraperInfos();
    m_meta.supportedEpisodeDetails = {EpisodeScraperInfo::Title};

    m_meta.supportedSeasonOrders = {SeasonOrder::Aired};
    m_meta.supportedLanguages = {Locale::English};
    m_meta.defaultLocale = Locale::English;
}

const EmptyTv::ScraperMeta& EmptyTv::meta() const
{
    return m_meta;
}

void EmptyTv::initialize()
{
    QTimer::singleShot(0, this, [this]() { emit initialized(true, this); });
}

bool EmptyTv::isInitialized() const
{
    return true;
}

ShowSearchJob* EmptyTv::search(ShowSearchJob::Config config)
{
    return new EmptyShowSearchJob(config, this);
}

ShowScrapeJob* EmptyTv::loadShow(ShowScrapeJob::Config config)
{
    return new EmptyShowScrapeJob(config, this);
}

SeasonScrapeJob* EmptyTv::loadSeasons(SeasonScrapeJob::Config config)
{
    return new EmptySeasonScrapeJob(config, this);
}

EpisodeScrapeJob* EmptyTv::loadEpisode(EpisodeScrapeJob::Config config)
{
    return new EmptyEpisodeScrapeJob(config, this);
}

EmptyShowSearchJob::EmptyShowSearchJob(ShowSearchJob::Config _config, QObject* parent) : ShowSearchJob(_config, parent)
{
}

void EmptyShowSearchJob::doStart()
{
    emitFinished();
}

EmptyShowScrapeJob::EmptyShowScrapeJob(ShowScrapeJob::Config _config, QObject* parent) : ShowScrapeJob(_config, parent)
{
}

void EmptyShowScrapeJob::doStart()
{
    QTimer::singleShot(0, this, [this]() { emitFinished(); });
}

EmptySeasonScrapeJob::EmptySeasonScrapeJob(SeasonScrapeJob::Config _config, QObject* parent) :
    SeasonScrapeJob(_config, parent)
{
}

void EmptySeasonScrapeJob::doStart()
{
    QTimer::singleShot(0, this, [this]() { emitFinished(); });
}

EmptyEpisodeScrapeJob::EmptyEpisodeScrapeJob(EpisodeScrapeJob::Config _config, QObject* parent) :
    EpisodeScrapeJob(_config, parent)
{
}

void EmptyEpisodeScrapeJob::doStart()
{
    QTimer::singleShot(0, this, [this]() { emitFinished(); });
}

} // namespace scraper
} // namespace mediaelch
