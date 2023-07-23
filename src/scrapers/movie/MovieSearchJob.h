#pragma once

#include "data/Locale.h"
#include "scrapers/ScraperError.h"
#include "scrapers/ScraperResult.h"
#include "scrapers/movie/MovieIdentifier.h"
#include "utils/Meta.h"
#include "workers/Job.h"

#include <QDate>
#include <QObject>
#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief A movie search request resolved by a scraper.
class MovieSearchJob : public worker::Job
{
    Q_OBJECT

public:
    /// \brief Configuration object for a movie search.
    struct Config
    {
        /// \brief The search string
        QString query;
        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        Locale locale = Locale::English;
        bool includeAdult = false;
    };

    /// \brief Search result of a movie search request.
    struct Result
    {
        /// \brief Title shown to the user.
        QString title;
        /// \brief Release date of the movie.
        /// \details Date is used for showing the year behind the search result.
        QDate released;
        /// \brief Scraper specific identifier that may be used for scraping a movie.
        /// \details The identifier can be passed to a load job.
        MovieIdentifier identifier;
    };

public:
    /// \brief Extract the title and year from a search query.
    /// \details This function checks for common patterns and extract the title
    ///          and year if a pattern matches.
    /// \returns Title/Year pair if a pattern matched, empty string pair otherwise.
    static QPair<QString, QString> extractTitleAndYear(const QString& query);

public:
    /// \brief Create a movie search.
    explicit MovieSearchJob(Config config, QObject* parent = nullptr);
    ~MovieSearchJob() override = default;

public:
    ELCH_NODISCARD const Config& config() const;
    ELCH_NODISCARD const ScraperError& scraperError() const;
    ELCH_NODISCARD const QVector<MovieSearchJob::Result>& results() const;

signals:
    /// \brief   Signal emitted when the search() request has finished.
    /// \details A simple wrapper around finished() to avoid static_asserts
    ///          from Job* to MovieSearchJob*.
    ///          Use hasError() and results() to know whether the request was successful.
    void searchFinished(mediaelch::scraper::MovieSearchJob* searchJob, QPrivateSignal);

protected:
    void setScraperError(ScraperError error);

protected:
    QVector<MovieSearchJob::Result> m_results;

private:
    ScraperError m_scraperError;
    const Config m_config;
};

QVector<ScraperSearchResult> toOldScraperSearchResult(const QVector<MovieSearchJob::Result>& searchResults);

} // namespace scraper
} // namespace mediaelch
