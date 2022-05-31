#include "scrapers/tv_show/empty/EmptyTvScraper.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

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

void EmptyEpisodeScrapeJob::start()
{
    QTimer::singleShot(0, this, [this]() { emit sigFinished(this); });
}

} // namespace scraper
} // namespace mediaelch
