#pragma once

#include "data/Locale.h"
#include "data/tv_show/EpisodeMap.h"
#include "data/tv_show/SeasonNumber.h"
#include "data/tv_show/SeasonOrder.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/tv_show/ShowIdentifier.h"
#include "utils/Meta.h"
#include "workers/Job.h"

#include <QObject>
#include <QSet>
#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief Load episodes of the given seasons.
class SeasonScrapeJob : public worker::Job
{
    Q_OBJECT

public:
    /// \brief Configuration object for a TV show scrape job.
    struct Config
    {
        Config(ShowIdentifier _identifier,
            Locale _locale,
            QSet<SeasonNumber> _seasons,
            SeasonOrder _seasonOrder,
            QSet<EpisodeScraperInfo> _details) :
            showIdentifier{std::move(_identifier)},
            locale{std::move(_locale)},
            seasons{std::move(_seasons)},
            seasonOrder{_seasonOrder},
            details{std::move(_details)}
        {
        }

        /// \brief A string that can be consumed by the TV show scraper.
        /// \details It is used to uniquely identify the TV show. May be an IMDb ID in
        ///          string representation or an URL.
        ShowIdentifier showIdentifier;

        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        Locale locale = Locale::English;

        /// \brief Set of seasons that whose episodes shall be loaded.
        QSet<SeasonNumber> seasons;

        /// \brief Order of episodes when loading seasons.
        SeasonOrder seasonOrder;

        /// \brief Details to be loaded using the scraper.
        QSet<EpisodeScraperInfo> details;

        /// \brief Returns true if all seasons should be loaded.
        bool shouldLoadAllSeasons() const { return seasons.isEmpty(); }
    };

public:
    SeasonScrapeJob(Config config, QObject* parent = nullptr);
    ~SeasonScrapeJob() override = default;

public:
    ELCH_NODISCARD const EpisodeMap& episodes() const { return m_episodes; }

    ELCH_NODISCARD const Config& config() const { return m_config; }

    ELCH_NODISCARD const ScraperError& scraperError() const;

signals:
    /// \brief   Signal emitted when the search() request has finished.
    /// \details A simple wrapper around finished() to avoid static_asserts
    ///          from Job* to SeasonScrapeJob*.
    ///          Use hasError() and tvShow() to know whether the request was successful.
    void loadFinished(SeasonScrapeJob* scrapeJob, QPrivateSignal);

protected:
    void setScraperError(ScraperError error);

protected:
    EpisodeMap m_episodes;

private:
    ScraperError m_scraperError;
    const Config m_config;
};

} // namespace scraper
} // namespace mediaelch
