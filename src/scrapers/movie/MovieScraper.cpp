#include "scrapers/movie/MovieScraper.h"

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

MovieScraper::MovieScraper(QObject* parent) : QObject(parent)
{
}

} // namespace scraper
} // namespace mediaelch
