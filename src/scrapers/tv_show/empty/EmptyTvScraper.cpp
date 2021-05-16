#include "scrapers/tv_show/empty/EmptyTvScraper.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

EmptyShowSearchJob::EmptyShowSearchJob(ShowSearchJob::Config _config, QObject* parent) : ShowSearchJob(_config, parent)
{
}

void EmptyShowSearchJob::start()
{
    QTimer::singleShot(0, [this]() { emit sigFinished(this); });
}

EmptyShowScrapeJob::EmptyShowScrapeJob(ShowScrapeJob::Config _config, QObject* parent) : ShowScrapeJob(_config, parent)
{
}

void EmptyShowScrapeJob::start()
{
    QTimer::singleShot(0, [this]() { emit sigFinished(this); });
}

EmptySeasonScrapeJob::EmptySeasonScrapeJob(SeasonScrapeJob::Config _config, QObject* parent) :
    SeasonScrapeJob(_config, parent)
{
}

void EmptySeasonScrapeJob::start()
{
    QTimer::singleShot(0, [this]() { emit sigFinished(this); });
}

EmptyEpisodeScrapeJob::EmptyEpisodeScrapeJob(EpisodeScrapeJob::Config _config, QObject* parent) :
    EpisodeScrapeJob(_config, parent)
{
}

void EmptyEpisodeScrapeJob::start()
{
    QTimer::singleShot(0, [this]() { emit sigFinished(this); });
}

} // namespace scraper
} // namespace mediaelch
