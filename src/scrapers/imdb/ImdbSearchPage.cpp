#include "ImdbSearchPage.h"

#include "log/Log.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

QVector<ImdbSearchPage::SearchResult> ImdbSearchPage::parseSuggestResponse(const QString& json,
    const QStringList& typeFilter)
{
    // Suggest API response format:
    // {"d":[{"i":{"imageUrl":"..."},"id":"tt2277860","l":"Finding Dory","q":"feature","s":"Ellen DeGeneres","y":2016}]}
    // Fields: id=IMDB ID, l=title, y=year, q=type (feature, tvSeries, etc.), s=stars summary

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(generic) << "[ImdbSearchPage] JSON parse error:" << parseError.errorString();
        return {};
    }

    QVector<SearchResult> results;
    const QJsonArray items = doc.object().value("d").toArray();

    for (const QJsonValue& item : items) {
        const QJsonObject obj = item.toObject();
        const QString id = obj.value("id").toString();

        // Only include title results (tt* IDs), skip name results (nm*)
        if (!id.startsWith("tt")) {
            continue;
        }

        // Filter by type if requested
        if (!typeFilter.isEmpty()) {
            const QString type = obj.value("qid").toString();
            if (!typeFilter.contains(type, Qt::CaseInsensitive)) {
                continue;
            }
        }

        SearchResult result;
        result.identifier = id;
        result.title = obj.value("l").toString();
        const int year = obj.value("y").toInt(0);
        if (year > 0) {
            result.released = QDate(year, 1, 1);
        }
        results.push_back(std::move(result));
    }

    return results;
}

} // namespace scraper
} // namespace mediaelch
