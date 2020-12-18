#pragma once

#include "data/Locale.h"
#include "globals/Meta.h"
#include "globals/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/tv_show/ShowIdentifier.h"
#include "tv_shows/EpisodeMap.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/SeasonOrder.h"

#include <QObject>
#include <QSet>
#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief Load episodes of the given seasons.
class SeasonScrapeJob : public QObject
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
    virtual ~SeasonScrapeJob() = default;

    virtual void execute() = 0;

public:
    ELCH_NODISCARD const EpisodeMap& episodes() const { return m_episodes; }

    ELCH_NODISCARD const Config& config() const { return m_config; }

    ELCH_NODISCARD bool hasError() const;
    ELCH_NODISCARD const ScraperError& error() const;

signals:
    /// \brief Signal emitted when the scrape job has finished.
    ///
    /// Use hasError() and tvShow() to know whether the request was successful.
    void sigFinished(SeasonScrapeJob* scrapeJob);

    /// \brief Signals a download progress. Useful if a scraper has to load
    ///        data from multiple sites or sends multiple requests.
    void sigProgress(int progress, int max);

protected:
    EpisodeMap m_episodes;
    const Config m_config;
    ScraperError m_error;
};

} // namespace scraper
} // namespace mediaelch
