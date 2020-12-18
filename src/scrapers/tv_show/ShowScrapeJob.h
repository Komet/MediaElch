#pragma once

#include "data/Locale.h"
#include "globals/Meta.h"
#include "globals/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/tv_show/ShowIdentifier.h"

#include <QObject>

class TvShow;

namespace mediaelch {
namespace scraper {
class ShowScrapeJob : public QObject
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
    virtual ~ShowScrapeJob() = default;

    virtual void execute() = 0;

public:
    ELCH_NODISCARD TvShow& tvShow() { return *m_tvShow; }
    ELCH_NODISCARD const TvShow& tvShow() const { return *m_tvShow; }

    ELCH_NODISCARD const Config& config() const { return m_config; }

    ELCH_NODISCARD bool hasError() const;
    ELCH_NODISCARD const ScraperError& error() const;

signals:
    /// \brief Signal emitted when the scrape job has finished.
    ///
    /// Use hasError() and tvShow() to know whether the request was successful.
    void sigFinished(ShowScrapeJob* scrapeJob);

    /// \brief Signals a download progress. Useful if a scraper has to load
    ///        data from multiple sites or sends multiple requests.
    void sigProgress(int progress, int max);

protected:
    TvShow* m_tvShow = nullptr;
    const Config m_config;
    ScraperError m_error;
};

} // namespace scraper
} // namespace mediaelch
