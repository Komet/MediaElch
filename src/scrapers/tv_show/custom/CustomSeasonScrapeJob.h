#pragma once

#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/custom/CustomTvScraperConfig.h"

#include <QMutex>
#include <QString>

namespace mediaelch {
namespace scraper {

class CustomSeasonScrapeJob : public SeasonScrapeJob
{
    Q_OBJECT

public:
    CustomSeasonScrapeJob(CustomTvScraperConfig customConfig, Config config, QObject* parent = nullptr);
    ~CustomSeasonScrapeJob() override = default;
    void execute() override;

private slots:
    void onTmdbShowLoaded(ShowScrapeJob* job);

private:
    /// \brief Load the season using the given show identifier from the specified scraper.
    void loadWithScraper(const QString& scraperId, const ShowIdentifier& identifier);
    /// \brief   Checks if all scrapers have finished.
    /// \details Thread safe check that all scrapers have finished. If all have finished,
    ///          sigFinished() is called.
    void decreaseCounterAndCheckIfFinished();
    /// \brief   Get the configuration for the given scraper.
    /// \details Uses the internal scrape job configuration but changes the scraper's language
    ///          according to MediaElch's settings.  So each scraper configuration may be
    ///          different in the language (and identifier of course).
    SeasonScrapeJob::Config configFor(const QString& scraperId, const ShowIdentifier& id);
    /// \brief Get the scraper's locale from MediaElch's settings.
    mediaelch::Locale localeFor(const QString& scraperId) const;

private:
    CustomTvScraperConfig m_customConfig;

    QMutex m_loadMutex;
    int m_loadCounter = 0;
};

} // namespace scraper
} // namespace mediaelch
