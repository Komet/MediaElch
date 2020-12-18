#include "scrapers/tv_show/ShowSearchJob.h"

#include <QRegularExpression>

namespace mediaelch {
namespace scraper {


QPair<QString, QString> ShowSearchJob::extractTitleAndYear(const QString& query)
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

ShowSearchJob::ShowSearchJob(ShowSearchJob::Config config, QObject* parent) :
    QObject(parent), m_config{std::move(config)}
{
}

const ShowSearchJob::Config& ShowSearchJob::config() const
{
    return m_config;
}

bool ShowSearchJob::hasError() const
{
    return m_error.hasError();
}


const ScraperError& ShowSearchJob::error() const
{
    return m_error;
}

const QVector<ShowSearchJob::Result>& ShowSearchJob::results() const
{
    return m_results;
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
