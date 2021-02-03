#pragma once

#include "data/Locale.h"
#include "globals/Meta.h"
#include "globals/ScraperInfos.h"
#include "scrapers/ScraperInterface.h"
#include "scrapers/movie/MovieIdentifier.h"

#include <QObject>

class Movie;

namespace mediaelch {
namespace scraper {

class MovieScrapeJob : public QObject
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
        Locale locale = Locale::English;

        /// \brief movie details to be loaded using the scraper.
        QSet<MovieScraperInfo> details;
    };

public:
    MovieScrapeJob(Config config, QObject* parent = nullptr);
    virtual ~MovieScrapeJob() = default;

    virtual void execute() = 0;

public:
    ELCH_NODISCARD Movie& movie() { return *m_movie; }
    ELCH_NODISCARD const Movie& movie() const { return *m_movie; }

    ELCH_NODISCARD const Config& config() const { return m_config; }

    ELCH_NODISCARD bool hasError() const;
    ELCH_NODISCARD const ScraperError& error() const;

signals:
    /// \brief Signal emitted when the scrape job has finished.
    ///
    /// Use hasError() and movie() to know whether the request was successful.
    void sigFinished(MovieScrapeJob* scrapeJob);

protected:
    Movie* m_movie = nullptr;
    const Config m_config;
    ScraperError m_error;
};

} // namespace scraper
} // namespace mediaelch
