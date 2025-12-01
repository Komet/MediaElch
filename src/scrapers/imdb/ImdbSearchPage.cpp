#include "ImdbSearchPage.h"

#include <QRegularExpression>

#include "scrapers/ScraperUtils.h"

namespace mediaelch {
namespace scraper {

QVector<ImdbSearchPage::SearchResult> ImdbSearchPage::parseSearch(const QString& html)
{
    // Search result table from "https://www.imdb.com/search/title/?title=..."
    // The results may contain the user's locale, e.g. `/de/title/…`.
    static const QRegularExpression rx(R"(<a href="/(?:\w{2,4}/)?title/(tt[\d]+)/[^>]+>(.+)</a>.*(\d{4})[–<])",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);
    // Entries are numbered: Remove Number.
    static const QRegularExpression listNo(
        R"(^\d+\.\s+)", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);

    QVector<SearchResult> results;
    QRegularExpressionMatchIterator matches = rx.globalMatch(html);
    QRegularExpressionMatch match;
    while (matches.hasNext()) {
        match = matches.next();
        if (match.hasMatch()) {
            QString title = normalizeFromHtml(match.captured(2));
            title.remove(listNo);
            SearchResult result;
            result.title = title;
            result.identifier = match.captured(1);
            result.released = QDate::fromString(match.captured(3), "yyyy");
            results.push_back(std::move(result));
        }
    }

    return results;
}

} // namespace scraper
} // namespace mediaelch
