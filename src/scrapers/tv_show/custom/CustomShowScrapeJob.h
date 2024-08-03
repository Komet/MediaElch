#pragma once

#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/custom/CustomTvScraperConfiguration.h"

#include <QMutex>
#include <QString>

namespace mediaelch {
namespace scraper {

class CustomShowScrapeJob : public ShowScrapeJob
{
    Q_OBJECT

public:
    CustomShowScrapeJob(CustomTvScraperConfiguration& customConfig, Config config, QObject* parent = nullptr);
    ~CustomShowScrapeJob() override = default;
    void doStart() override;

private slots:
    void onTmdbLoaded(mediaelch::scraper::ShowScrapeJob* job);

private:
    /// \brief Load the show using the given show identifier from the specified scraper.
    void loadWithScraper(const QString& scraperId, const ShowIdentifier& identifier);
    /// \brief   Checks if all scrapers have finished.
    /// \details Thread safe check that all scrapers have finished. If all have finished,
    ///          sigFinished() is called.
    void decreaseCounterAndCheckIfFinished();
    /// \brief   Get the configuration for the given scraper.
    /// \details Uses the internal scrape job configuration but changes the scraper's language
    ///          according to MediaElch's settings.  So each scraper configuration may be
    ///          different in the language (and identifier of course).
    ShowScrapeJob::Config configFor(const QString& scraperId, const ShowIdentifier& id);
    /// \brief Get the scraper's locale from MediaElch's settings.
    mediaelch::Locale localeFor(const QString& scraperId) const;

private:
    CustomTvScraperConfiguration& m_customConfig;

    QMutex m_loadMutex;
    int m_loadCounter = 0;
};

} // namespace scraper
} // namespace mediaelch
