#pragma once

#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/custom/CustomTvScraperConfiguration.h"

#include <QString>

namespace mediaelch {
namespace scraper {

class CustomEpisodeScrapeJob : public EpisodeScrapeJob
{
    Q_OBJECT

public:
    CustomEpisodeScrapeJob(CustomTvScraperConfiguration& customConfig, Config config, QObject* parent = nullptr);
    ~CustomEpisodeScrapeJob() override = default;
    void doStart() override;

private slots:
    void onTmdbLoaded(mediaelch::scraper::EpisodeScrapeJob* job);

private:
    /// \brief Load the episode using the given episode identifier from the specified scraper.
    void loadWithScraper(const QString& scraperId, const EpisodeIdentifier& identifier);
    /// \brief   Checks if all scrapers have finished.
    /// \details Thread safe check that all scrapers have finished. If all have finished,
    ///          sigFinished() is called.
    void decreaseCounterAndCheckIfFinished();
    /// \brief   Get the configuration for the given scraper.
    /// \details Uses the internal scrape job configuration but changes the scraper's language
    ///          according to MediaElch's settings.  So each scraper configuration may be
    ///          different in the language (and identifier of course).
    EpisodeScrapeJob::Config configFor(const QString& scraperId, const EpisodeIdentifier& id);
    /// \brief Get the scraper's locale from MediaElch's settings.
    mediaelch::Locale localeFor(const QString& scraperId) const;

private:
    CustomTvScraperConfiguration& m_customConfig;
    int m_loadCounter = 0;
};

} // namespace scraper
} // namespace mediaelch
