#pragma once

#include "data/Locale.h"
#include "globals/Meta.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperError.h"
#include "scrapers/movie/MovieIdentifier.h"

#include <QDate>
#include <QObject>
#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief A movie search request resolved by a scraper.
class MovieSearchJob : public QObject
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
        /// \details The identifier can be passed to scrape().
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
    virtual ~MovieSearchJob() = default;

    virtual void execute() = 0;

public:
    ELCH_NODISCARD const Config& config() const;
    ELCH_NODISCARD bool hasError() const;
    ELCH_NODISCARD const ScraperError& error() const;
    ELCH_NODISCARD const QVector<MovieSearchJob::Result>& results() const;

signals:
    /// \brief Signal emitted when the search() request has finished.
    ///
    /// Use hasError() and results() to know whether the request was successful.
    void sigFinished(MovieSearchJob* searchJob);

protected:
    QVector<MovieSearchJob::Result> m_results;
    ScraperError m_error;

private:
    const Config m_config;
};

QVector<ScraperSearchResult> toOldScraperSearchResult(const QVector<MovieSearchJob::Result>& searchResults);

} // namespace scraper
} // namespace mediaelch
