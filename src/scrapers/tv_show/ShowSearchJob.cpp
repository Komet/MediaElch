#include "scrapers/tv_show/ShowSearchJob.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {


QPair<QString, QString> ShowSearchJob::extractTitleAndYear(const QString& query)
{
    QVector<QRegularExpression> yearRegEx;
    yearRegEx << QRegularExpression(R"(^(.+) \((\d{4})\)$)") //
              << QRegularExpression(R"(^(.+) (\d{4})$)")     //
              << QRegularExpression(R"(^(.+) - (\d{4})$)");

    for (auto& rxYear : yearRegEx) {
        QRegularExpressionMatch match = rxYear.match(query);
        if (match.hasMatch()) {
            QString searchTitle = match.captured(1);
            QString searchYear = match.captured(2);
            return {searchTitle, searchYear};
        }
    }
    return {query, QString{}};
}

ShowSearchJob::ShowSearchJob(ShowSearchJob::Config config, QObject* parent) :
    worker::Job(parent), m_config{std::move(config)}
{
    // Wrapper to avoid static_assert calls.
    connect(this, &Job::finished, this, [this]() { emit searchFinished(this, QPrivateSignal{}); });

    // TODO: Change to true / remove once all usages of ShowSearchJob are updated.
    setAutoDelete(false);
}

const ShowSearchJob::Config& ShowSearchJob::config() const
{
    return m_config;
}

const ScraperError& ShowSearchJob::scraperError() const
{
    return m_scraperError;
}

const QVector<ShowSearchJob::Result>& ShowSearchJob::results() const
{
    return m_results;
}

void ShowSearchJob::setScraperError(ScraperError error)
{
    m_scraperError = std::move(error);
    setError(static_cast<int>(m_scraperError.error));
    setErrorString(m_scraperError.message);
    setErrorText(m_scraperError.technical);
}

QVector<ScraperSearchResult> toOldScraperSearchResult(const QVector<ShowSearchJob::Result>& searchResults)
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

bool ShowSearchJob::Result::isValid() const
{
    return !identifier.str().isEmpty() && !title.isEmpty();
}


} // namespace scraper
} // namespace mediaelch
