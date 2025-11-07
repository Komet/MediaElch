#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/tv_show/EpisodeIdentifier.h"
#include "utils/Meta.h"
#include "workers/Job.h"

#include <QObject>
#include <QSet>

class TvShowEpisode;

namespace mediaelch {
namespace scraper {

class EpisodeScrapeJob : public worker::Job
{
    Q_OBJECT

public:
    /// \brief Configuration object for a TV show scrape job.
    struct Config
    {
        Config(EpisodeIdentifier _identifier, Locale _locale, QSet<EpisodeScraperInfo> _details) :
            identifier{std::move(_identifier)}, locale{std::move(_locale)}, details{std::move(_details)}
        {
        }

        /// \brief An identifier that can be consumed by the episode scraper.
        /// \details It is used to uniquely identify the episode. May be a TvDb ID in
        ///          string representation or an URL or a combination of season/episode
        ///          number and TvShow id.
        EpisodeIdentifier identifier;

        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        Locale locale = Locale::English;

        /// \brief   Details to be loaded using the scraper.
        /// \details The scraper may set more details, as this field is only used for optimizations
        ///          such as reducing the number of HTTP requests.
        QSet<EpisodeScraperInfo> details;
    };

public:
    EpisodeScrapeJob(Config config, QObject* parent = nullptr);
    virtual ~EpisodeScrapeJob() = default;

    ELCH_NODISCARD TvShowEpisode& episode() { return *m_episode; }
    ELCH_NODISCARD const Config& config() { return m_config; }

    ELCH_NODISCARD const ScraperError& scraperError() const;

signals:
    /// \brief   Signal emitted when the scrape job has finished.
    /// \details A simple wrapper around finished() to avoid dynamic_casts
    ///          from Job* to EpisodeScrapeJob*.
    ///          Use hasError() and episode() to know whether the request was successful.
    void loadFinished(mediaelch::scraper::EpisodeScrapeJob* scrapeJob, QPrivateSignal);

protected:
    void setScraperError(ScraperError error);

protected:
    /// \brief Episode to be filled by scraper; owned by scrape job
    TvShowEpisode* m_episode = nullptr;

private:
    ScraperError m_scraperError;
    const Config m_config;
};

} // namespace scraper
} // namespace mediaelch
