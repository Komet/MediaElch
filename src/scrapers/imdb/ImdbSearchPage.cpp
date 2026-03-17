#include "ImdbSearchPage.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "log/Log.h"

namespace mediaelch {
namespace scraper {

QVector<ImdbSearchPage::SearchResult> ImdbSearchPage::parseSearch(const QString& html)
{
    QVector<SearchResult> results;

    // IMDb embeds search results as JSON in a <script id="__NEXT_DATA__"> tag.
    // Extract and parse the JSON rather than scraping the HTML with regex.
    static const QRegularExpression rx(
        R"re(<script id="__NEXT_DATA__" type="application/json">(.*)</script>)re",
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatch match = rx.match(html);
    if (!match.hasMatch()) {
        qCWarning(generic) << "[ImdbSearch] Could not find __NEXT_DATA__ JSON in search page";
        return results;
    }

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(match.captured(1).toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(generic) << "[ImdbSearch] Failed to parse __NEXT_DATA__ JSON:" << parseError.errorString();
        return results;
    }

    // Path: props.pageProps.searchResults.titleResults.titleListItems
    const QJsonArray items = doc.object()
                                 .value("props")
                                 .toObject()
                                 .value("pageProps")
                                 .toObject()
                                 .value("searchResults")
                                 .toObject()
                                 .value("titleResults")
                                 .toObject()
                                 .value("titleListItems")
                                 .toArray();

    for (const QJsonValue& item : items) {
        const QJsonObject obj = item.toObject();
        const QString titleId = obj.value("titleId").toString();
        const QString titleText = obj.value("titleText").toString();
        const int releaseYear = obj.value("releaseYear").toInt(0);

        if (titleId.isEmpty() || titleText.isEmpty()) {
            continue;
        }

        SearchResult result;
        result.identifier = titleId;
        result.title = titleText;
        if (releaseYear > 0) {
            result.released = QDate(releaseYear, 1, 1);
        }
        results.push_back(std::move(result));
    }

    qCDebug(generic) << "[ImdbSearch] Parsed" << results.size() << "results from __NEXT_DATA__ JSON";
    return results;
}

} // namespace scraper
} // namespace mediaelch
