#pragma once

#include "scrapers/ScraperInfos.h"
#include "scrapers/movie/MovieScrapeJob.h"

#include <QHash>
#include <QMap>
#include <QVector>

namespace mediaelch {
namespace scraper {

class MovieScraper;

class CustomMovieScrapeJob : public MovieScrapeJob
{
    Q_OBJECT

public:
    struct CustomScraperConfig
    {
        /// \brief Actual custom scraper configuration.
        QHash<MovieScraper*, MovieScrapeJob::Config> scraperMap;
    };

public:
    explicit CustomMovieScrapeJob(CustomScraperConfig _config, QObject* parent = nullptr);
    ~CustomMovieScrapeJob() override = default;

    void doStart() override;

private slots:
    /// \brief   Signal handler when a sub-scraper is finished.
    /// \details The custom movie scraper only dispatches loading to other
    ///          scrapers.  If one of them is finished, this slot is called.
    ///          If all scrapers are finished, loadFinished() is emitted.
    void onScraperFinished(mediaelch::scraper::MovieScrapeJob* scrapeJob);

private:
    CustomScraperConfig m_customScraperConfig;
    QVector<MovieScrapeJob*> m_jobs;
};

} // namespace scraper
} // namespace mediaelch
