#include "scrapers/tv_show/tmdb/TmdbTvShowSearchJob.h"

#include "scrapers/tmdb/TmdbApi.h"

#include <QJsonArray>

namespace mediaelch {
namespace scraper {

TmdbTvShowSearchJob::TmdbTvShowSearchJob(TmdbApi& api, ShowSearchJob::Config _config, QObject* parent) :
    ShowSearchJob(_config, parent), m_api{api}
{
}

void TmdbTvShowSearchJob::execute()
{
    if (config().query.isEmpty()) {
        // searching without a query results in a network error
        emit sigFinished(this);
        return;
    }

    QString title = ShowSearchJob::extractTitleAndYear(config().query).first;

    // TODO: TMDb supports search by year. We could add it as well.
    m_api.searchForShow(config().locale, title, config().includeAdult, [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            m_results = parseSearch(json);
        } else {
            m_error = error;
        }
        emit sigFinished(this);
    });
}

QVector<ShowSearchJob::Result> TmdbTvShowSearchJob::parseSearch(const QJsonDocument& json)
{
    QVector<ShowSearchJob::Result> results;

    QJsonObject searchObj = json.object();

    QJsonArray resultArray;
    if (searchObj.contains("results")) {
        // TV search array
        resultArray = searchObj["results"].toArray();

    } else if (searchObj.contains("tv_results")) {
        // The /find/ results array
        resultArray = searchObj["tv_results"].toArray();
    }

    for (const QJsonValue& val : asConst(resultArray)) {
        auto searchResult = parseSingleSearchObject(val.toObject());
        if (!searchResult.identifier.str().isEmpty()) {
            results << searchResult;
        }
    }

    // It is possible that the returned JSON document is a specific show's page.
    if (results.isEmpty() && searchObj.contains("name")) {
        auto searchResult = parseSingleSearchObject(searchObj);
        if (!searchResult.identifier.str().isEmpty()) {
            results << searchResult;
        }
    }

    return results;
}

ShowSearchJob::Result TmdbTvShowSearchJob::parseSingleSearchObject(const QJsonObject& json)
{
    ShowSearchJob::Result result;
    result.title = json["name"].toString();
    result.identifier = ShowIdentifier(QString::number(json["id"].toInt()));
    result.released = QDate::fromString(json["first_air_date"].toString(), "yyyy-MM-dd");
    return result;
}


} // namespace scraper
} // namespace mediaelch
