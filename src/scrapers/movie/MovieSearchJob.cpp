#include "scrapers/movie/MovieSearchJob.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {


QPair<QString, QString> MovieSearchJob::extractTitleAndYear(const QString& query)
{
    QVector<QRegularExpression> yearRegEx;
    yearRegEx << QRegularExpression(R"(^(.+) \((\d{4})\)$)") //
              << QRegularExpression(R"(^(.+) (\d{4})$)")     //
              << QRegularExpression(R"(^(.+) - (\d{4})$)");

    for (auto& rxYear : yearRegEx) {
        // minimal matching
        rxYear.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
        auto match = rxYear.match(query);
        if (match.hasMatch()) {
            QString searchTitle = match.captured(0);
            QString searchYear = match.captured(1);
            return {searchTitle, searchTitle};
        }
    }
    return {};
}

MovieSearchJob::MovieSearchJob(MovieSearchJob::Config config, QObject* parent) :
    worker::Job(parent), m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit searchFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of ShowSearhJob are updated.
    setAutoDelete(false);
}

const MovieSearchJob::Config& MovieSearchJob::config() const
{
    return m_config;
}

const ScraperError& MovieSearchJob::scraperError() const
{
    return m_scraperError;
}

const QVector<MovieSearchJob::Result>& MovieSearchJob::results() const
{
    return m_results;
}

void MovieSearchJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}

QVector<ScraperSearchResult> toOldScraperSearchResult(const QVector<MovieSearchJob::Result>& searchResults)
{
    QVector<ScraperSearchResult> results;
    for (const auto& searchResult : searchResults) {
        ScraperSearchResult result;
        result.id = searchResult.identifier.str();
        result.name = searchResult.title;
        result.released = searchResult.released;
        results.push_back(result);
    }
    return results;
}

} // namespace scraper
} // namespace mediaelch
