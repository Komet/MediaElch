#include "scrapers/movie/MovieSearchJob.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {


QPair<QString, QString> MovieSearchJob::extractTitleAndYear(const QString& query)
{
    QVector<QRegularExpression> yearRegEx;
    yearRegEx << QRegularExpression(R"(^(.*) \((\d{4})\)$)") //
              << QRegularExpression(R"(^(.*) (\d{4})$)")     //
              << QRegularExpression(R"(^(.*) - (\d{4})$)");

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
    QObject(parent), m_config{std::move(config)}
{
}

const MovieSearchJob::Config& MovieSearchJob::config() const
{
    return m_config;
}

bool MovieSearchJob::hasError() const
{
    return m_error.hasError();
}


const ScraperError& MovieSearchJob::error() const
{
    return m_error;
}

const QVector<MovieSearchJob::Result>& MovieSearchJob::results() const
{
    return m_results;
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
