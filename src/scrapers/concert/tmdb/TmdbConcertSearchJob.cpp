#include "scrapers/concert/tmdb/TmdbConcertSearchJob.h"

#include "scrapers/tmdb/TmdbApi.h"

#include <QJsonArray>

namespace mediaelch {
namespace scraper {

TmdbConcertSearchJob::TmdbConcertSearchJob(TmdbApi& api, ConcertSearchJob::Config _config, QObject* parent) :
    ConcertSearchJob(_config, parent), m_api{api}
{
}

void TmdbConcertSearchJob::execute()
{
    m_api.searchForConcert(config().locale, config().query, [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            m_results = parseSearch(json);
        } else {
            m_error = error;
        }
        emit sigFinished(this);
    });
}

QVector<ConcertSearchJob::Result> TmdbConcertSearchJob::parseSearch(const QJsonDocument& json)
{
    QVector<ConcertSearchJob::Result> results;

    QJsonObject searchObj = json.object();

    QJsonArray resultArray;
    if (searchObj.contains("results")) {
        // Movie search array
        resultArray = searchObj["results"].toArray();

    } else if (searchObj.contains("movie_results")) {
        // The /find/ results array
        resultArray = searchObj["movie_results"].toArray();
    }

    for (const QJsonValue& val : asConst(resultArray)) {
        auto searchResult = parseSingleSearchObject(val.toObject());
        if (!searchResult.identifier.str().isEmpty()) {
            results << searchResult;
        }
    }

    // It is possible that the returned JSON document is a specific movie's page.
    if (results.isEmpty() && searchObj.contains("name")) {
        auto searchResult = parseSingleSearchObject(searchObj);
        if (!searchResult.identifier.str().isEmpty()) {
            results << searchResult;
        }
    }

    return results;
}

ConcertSearchJob::Result TmdbConcertSearchJob::parseSingleSearchObject(const QJsonObject& json)
{
    ConcertSearchJob::Result result;
    result.title = json["title"].toString();
    if (result.title.isEmpty()) {
        result.title = json["original_title"].toString();
    }
    result.identifier = ConcertIdentifier(QString::number(json["id"].toInt()));
    result.released = QDate::fromString(json["release_date"].toString(), "yyyy-MM-dd");

    return result;
}

} // namespace scraper
} // namespace mediaelch
