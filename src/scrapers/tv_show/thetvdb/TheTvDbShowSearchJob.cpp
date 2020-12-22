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
    if (config().query.isEmpty()) {
        // Searching for an empty string results in a network error.
        emit sigFinished(this);
        return;
    }
    m_api.searchForShow(config().locale, config().query, [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            m_results = parseSearch(json);

        } else if (!error.is404()) {
            // ignore 404 because this means that the search did not provide any results.
            m_error = error;
        }
        emit sigFinished(this);
    });
}

QVector<ShowSearchJob::Result> TheTvDbShowSearchJob::parseSearch(const QJsonDocument& json)
{
    QJsonObject parsedJson = json.object();
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
