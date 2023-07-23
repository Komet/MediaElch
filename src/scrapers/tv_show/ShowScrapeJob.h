#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/tv_show/ShowIdentifier.h"
#include "utils/Meta.h"
#include "workers/Job.h"

#include <QObject>

class TvShow;

namespace mediaelch {
namespace scraper {
class ShowScrapeJob : public worker::Job
{
    Q_OBJECT

public:
    /// \brief Configuration object for a TV show scrape job.
    struct Config
    {
        /// \brief A string that can be consumed by the TV show scraper.
        /// \details It is used to uniquely identify the TV show. May be an IMDb ID in
        ///          string representation or an URL.
        ShowIdentifier identifier;
        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        Locale locale = Locale::English;
        /// \brief TV show details to be loaded using the scraper.
        QSet<ShowScraperInfo> details;
    };

public:
    ShowScrapeJob(Config config, QObject* parent = nullptr);
    ~ShowScrapeJob() override = default;

public:
    ELCH_NODISCARD TvShow& tvShow() { return *m_tvShow; }
    ELCH_NODISCARD const TvShow& tvShow() const { return *m_tvShow; }

    ELCH_NODISCARD const Config& config() const { return m_config; }
    ELCH_NODISCARD const ScraperError& scraperError() const;

signals:
    /// \brief   Signal emitted when the scrape job has finished.
    /// \details A simple wrapper around finished() to avoid static_asserts
    ///          from Job* to ShowScrapeJob*.
    ///          Use hasError() and tvShow() to know whether the request was successful.
    void loadFinished(mediaelch::scraper::ShowScrapeJob* scrapeJob, QPrivateSignal);

protected:
    void setScraperError(ScraperError error);

protected:
    TvShow* m_tvShow = nullptr;

private:
    ScraperError m_scraperError;
    const Config m_config;
};

} // namespace scraper
} // namespace mediaelch
