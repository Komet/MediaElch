#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperError.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/movie/MovieIdentifier.h"
#include "utils/Meta.h"
#include "workers/Job.h"

#include <QDebug>
#include <QObject>

class Movie;

namespace mediaelch {
namespace scraper {

class MovieScrapeJob : public worker::Job
{
    Q_OBJECT

public:
    /// \brief Configuration object for a movie scrape job.
    struct Config
    {
        /// \brief A string that can be consumed by the movie scraper.
        /// \details It is used to uniquely identify the movie. May be an IMDb ID in
        ///          string representation or an URL.
        MovieIdentifier identifier;
        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        /// \details Default language is "no locale" to more easily detect issues and to make
        ///          it possible that movie scrapers can check against something to set defaults.
        Locale locale = Locale::NoLocale;
        /// \brief movie details to be loaded using the scraper.
        QSet<MovieScraperInfo> details;
    };

public:
    explicit MovieScrapeJob(Config config, QObject* parent = nullptr);
    ~MovieScrapeJob() override = default;

public:
    ELCH_NODISCARD Movie& movie() { return *m_movie; }
    ELCH_NODISCARD const Movie& movie() const { return *m_movie; }

    ELCH_NODISCARD const Config& config() const { return m_config; }
    ELCH_NODISCARD const ScraperError& scraperError() const;

signals:
    /// \brief   Signal emitted when the scrape job has finished.
    /// \details A simple wrapper around finished() to avoid static_asserts
    ///          from Job* to MovieScrapeJob*.
    ///          Use hasError() and movie() to know whether the request was successful.
    void loadFinished(mediaelch::scraper::MovieScrapeJob* scrapeJob, QPrivateSignal);

protected:
    void setScraperError(ScraperError error);

protected:
    Movie* m_movie = nullptr;

private:
    const Config m_config;
    ScraperError m_scraperError;
};

} // namespace scraper
} // namespace mediaelch

QDebug operator<<(QDebug debug, const mediaelch::scraper::MovieScrapeJob::Config& config);
