#include "scrapers/tv_show/tmdb/TmdbTvShowSearchJob.h"

#include "scrapers/api/TmdbApi.h"

#include <QJsonArray>

namespace mediaelch {
namespace scraper {

TmdbTvShowSearchJob::TmdbTvShowSearchJob(TmdbApi& api, ShowSearchJob::Config _config, QObject* parent) :
    ShowSearchJob(_config, parent), m_api{api}
{
}

void TmdbTvShowSearchJob::execute()
{
    m_api.searchForShow(
        config().locale, config().query, config().includeAdult, [this](QJsonDocument json, ScraperError error) {
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

    const QJsonArray resultArray = json.object()["results"].toArray();
    for (const QJsonValue& val : resultArray) {
        results << parseSingleSearchObject(val.toObject());
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
