#include "scrapers/tv_show/omdb/OmdbTvShowSearchJob.h"

#include "data/ImdbId.h"
#include "scrapers/omdb/OmdbApi.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

namespace {

QString stripYearSuffix(const QString& title)
{
    static const QRegularExpression yearSuffix(R"(\s*\(\d{4}\)$)");
    return QString(title).remove(yearSuffix).trimmed();
}

struct QueryAndYear
{
    QString query;
    int year = 0;
};

QueryAndYear extractYear(const QString& input)
{
    static const QRegularExpression trailingYear(R"(^(.+?)\s+(\d{4})$)");
    QRegularExpressionMatch match = trailingYear.match(input.trimmed());
    if (match.hasMatch()) {
        int year = match.captured(2).toInt();
        if (year >= 1888 && year <= 2100) {
            return {match.captured(1).trimmed(), year};
        }
    }
    return {input.trimmed(), 0};
}

} // namespace

namespace mediaelch {
namespace scraper {

OmdbTvShowSearchJob::OmdbTvShowSearchJob(OmdbApi& api, ShowSearchJob::Config _config, QObject* parent) :
    ShowSearchJob(_config, parent), m_api{api}
{
}

void OmdbTvShowSearchJob::doStart()
{
    if (ImdbId::isValidFormat(config().query)) {
        searchViaImdbId();
    } else {
        searchViaQuery();
    }
}

void OmdbTvShowSearchJob::searchViaImdbId()
{
    m_api.loadShow(ImdbId(config().query), [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }

        QJsonObject obj = json.object();
        ShowSearchJob::Result result;
        result.title = stripYearSuffix(obj.value("Title").toString());
        result.released = QDate::fromString(obj.value("Year").toString().left(4), "yyyy");
        result.identifier = ShowIdentifier(obj.value("imdbID").toString());
        m_results.push_back(std::move(result));

        emitFinished();
    });
}

void OmdbTvShowSearchJob::searchViaQuery()
{
    const auto [query, year] = extractYear(config().query);
    m_api.searchForShow(query, year, 1, [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            if (error.message == "Series not found!") {
                emitFinished();
                return;
            }
            setScraperError(error);
            emitFinished();
            return;
        }

        parseSearch(json);
        emitFinished();
    });
}

void OmdbTvShowSearchJob::parseSearch(const QJsonDocument& json)
{
    QJsonArray results = json.object().value("Search").toArray();
    for (const QJsonValue& val : results) {
        QJsonObject obj = val.toObject();
        ShowSearchJob::Result result;
        result.title = stripYearSuffix(obj.value("Title").toString());
        // OMDb TV Year format can be "2008–2013" or "2008–"
        result.released = QDate::fromString(obj.value("Year").toString().left(4), "yyyy");
        result.identifier = ShowIdentifier(obj.value("imdbID").toString());
        m_results.push_back(std::move(result));
    }
}

} // namespace scraper
} // namespace mediaelch
