#include "Search.h"

#include "globals/Globals.h"
#include "scrapers/tv_show/TheTvDb/ApiRequest.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

namespace thetvdb {

Search::Search(QString language, QObject* parent) : QObject(parent), m_apiRequest(language)
{
    setParent(parent);
}

void Search::search(QString searchStr)
{
    m_apiRequest.sendGetRequest(getShowSearchUrl(searchStr), [this](QString json) {
        const auto results = parseSearch(json);
        emit sigSearchDone(results);
    });
}

QVector<ScraperSearchResult> Search::parseSearch(const QString& json)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[TheTvDb][Search] Error parsing search:" << parseError.errorString();
        return {};
    }

    QVector<ScraperSearchResult> results;
    QJsonArray jsonResults = parsedJson.value("data").isArray() ? parsedJson.value("data").toArray()
                                                                : QJsonArray{parsedJson.value("data").toObject()};
    for (const QJsonValueRef showObjectValue : jsonResults) {
        const auto result = parseSingleSearchResult(showObjectValue.toObject());
        if (!result.name.isEmpty()) {
            results.append(result);
        }
    }

    return results;
}

ScraperSearchResult Search::parseSingleSearchResult(const QJsonObject& showObject)
{
    ScraperSearchResult result;
    result.name = showObject.value("seriesName").toString();
    result.id = QString::number(showObject.value("id").toInt());
    if (showObject.value("firstAired").isString()) {
        QString str = showObject.value("firstAired").toString();
        // TheTVDb month and day don't have a leading zero
        result.released = QDate::fromString(str, "yyyy-M-d");
    }
    return result;
}

QUrl Search::getShowSearchUrl(QString searchStr) const
{
    const QRegExp rxId("^id(\\d+)$");
    if (rxId.exactMatch(searchStr)) {
        return ApiRequest::getFullUrl("/series/" + rxId.cap(1));
    }

    QUrlQuery queries;
    queries.addQueryItem("name", searchStr);
    return ApiRequest::getFullUrl("/search/series?" + queries.toString());
}

} // namespace thetvdb
