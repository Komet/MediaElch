#include "scrapers/tv_show/thetvdb/TheTvDbShowSearchJob.h"

#include "globals/Globals.h"
#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>
#include <QString>
#include <QUrl>

namespace mediaelch {
namespace scraper {

TheTvDbShowSearchJob::TheTvDbShowSearchJob(TheTvDbApi& api, ShowSearchJob::Config config, QObject* parent) :
    ShowSearchJob(std::move(config), parent), m_api{api}
{
}

void TheTvDbShowSearchJob::execute()
{
    m_api.searchForShow(config().locale, config().query, [this](QString json) {
        m_results = parseSearch(json);
        emit sigFinished(this);
    });
}

QVector<ShowSearchJob::Result> TheTvDbShowSearchJob::parseSearch(const QString& json)
{
    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "[TheTvDbShowSearchJob] Error parsing search:" << parseError.errorString();
        return {};
    }

    QVector<ShowSearchJob::Result> results;
    QJsonArray jsonResults = parsedJson.value("data").isArray() ? parsedJson.value("data").toArray()
                                                                : QJsonArray{parsedJson.value("data").toObject()};
    for (const QJsonValueRef showObjectValue : jsonResults) {
        const auto result = parseSingleSearchResult(showObjectValue.toObject());
        if (!result.title.isEmpty()) {
            results.append(result);
        }
    }

    return results;
}

ShowSearchJob::Result TheTvDbShowSearchJob::parseSingleSearchResult(const QJsonObject& showObject)
{
    ShowSearchJob::Result result;
    result.title = showObject.value("seriesName").toString();
    result.identifier = ShowIdentifier(QString::number(showObject.value("id").toInt()));
    if (showObject.value("firstAired").isString()) {
        QString str = showObject.value("firstAired").toString();
        // TheTVDb month and day don't have a leading zero
        result.released = QDate::fromString(str, "yyyy-M-d");
    }
    return result;
}

} // namespace scraper
} // namespace mediaelch
