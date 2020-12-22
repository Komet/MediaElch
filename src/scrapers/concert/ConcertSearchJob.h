#pragma once

#include "data/Locale.h"
#include "globals/Meta.h"
#include "globals/ScraperResult.h"
#include "scrapers/ScraperError.h"
#include "scrapers/concert/ConcertIdentifier.h"

#include <QDate>
#include <QObject>
#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief A concert search request resolved by a scraper.
class ConcertSearchJob : public QObject
{
    Q_OBJECT

public:
    /// \brief Configuration object for a concert search.
    struct Config
    {
        /// \brief The search string
        QString query;
        /// \brief Language key for the scraper, e.g. "en-US", "de-DE", ...
        Locale locale = Locale::English;
    };

    /// \brief Search result of a concert search request.
    struct Result
    {
        /// \brief Title shown to the user.
        QString title;
        /// \brief Release date of the concert.
        /// \details Date is used for showing the year behind the search result.
        QDate released;
        /// \brief Scraper specific identifier that may be used for scraping a concert.
        /// \details The identifier can be passed to scrape().
        ConcertIdentifier identifier;
    };

public:
    /// \brief Extract the title and year from a search query.
    /// \details This function checks for common patterns and extract the title
    ///          and year if a pattern matches.
    /// \returns Title/Year pair if a pattern matched, empty string pair otherwise.
    static QPair<QString, QString> extractTitleAndYear(const QString& query);

public:
    /// \brief Create a concert search.
    explicit ConcertSearchJob(Config config, QObject* parent = nullptr);
    virtual ~ConcertSearchJob() = default;

    virtual void execute() = 0;

public:
    ELCH_NODISCARD const Config& config() const;
    ELCH_NODISCARD bool hasError() const;
    ELCH_NODISCARD const ScraperError& error() const;
    ELCH_NODISCARD const QVector<ConcertSearchJob::Result>& results() const;

signals:
    /// \brief Signal emitted when the search() request has finished.
    ///
    /// Use hasError() and results() to know whether the request was successful.
    void sigFinished(ConcertSearchJob* searchJob);

protected:
    QVector<ConcertSearchJob::Result> m_results;
    ScraperError m_error;

private:
    const Config m_config;
};

QVector<ScraperSearchResult> toOldScraperSearchResult(const QVector<ConcertSearchJob::Result>& searchResults);

} // namespace scraper
} // namespace mediaelch
