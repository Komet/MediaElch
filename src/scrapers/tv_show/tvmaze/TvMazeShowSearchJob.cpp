#include "scrapers/tv_show/tvmaze/TvMazeShowSearchJob.h"

#include "globals/Globals.h"
#include "scrapers/tv_show/tvmaze/TvMazeApi.h"

#include <QJsonArray>
#include <QObject>
#include <QString>
#include <QUrl>

namespace mediaelch {
namespace scraper {

TvMazeShowSearchJob::TvMazeShowSearchJob(TvMazeApi& api, ShowSearchJob::Config config, QObject* parent) :
    ShowSearchJob(std::move(config), parent), m_api{api}
{
}

void TvMazeShowSearchJob::execute()
{
    m_api.searchForShow(config().query, [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            m_results = parseSearch(json);
        } else {
            m_error = error;
        }
        emit sigFinished(this);
    });
}

QVector<ShowSearchJob::Result> TvMazeShowSearchJob::parseSearch(const QJsonDocument& json) const
{
    QVector<ShowSearchJob::Result> results;

    const QJsonArray resultArray = json.array();
    for (const QJsonValue& val : resultArray) {
        ShowSearchJob::Result result = parseSingleSearchObject(val.toObject()["show"].toObject());
        if (result.isValid()) {
            results.append(result);
        }
    }
    return results;
}

ShowSearchJob::Result TvMazeShowSearchJob::parseSingleSearchObject(const QJsonObject& json) const
{
    ShowSearchJob::Result result;

    const int id = json["id"].toInt(0);
    if (id > 0) {
        result.identifier = ShowIdentifier(TvMazeId(id));
    }

    result.title = json["name"].toString();
    result.released = QDate::fromString(json["premiered"].toString(), "yyyy-MM-dd");

    return result;
}

} // namespace scraper
} // namespace mediaelch
