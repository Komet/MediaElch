#include "scrapers/movie/omdb/OmdbMovieSearchJob.h"

#include "data/ImdbId.h"
#include "scrapers/omdb/OmdbApi.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

namespace {

/// \brief Strip trailing year suffix like " (2010)" from OMDb titles.
/// \details OMDb sometimes includes the year in the title (e.g. "Inception (2010)").
///          MediaElch adds the year separately, so we remove it to avoid duplication.
QString stripYearSuffix(const QString& title)
{
    static const QRegularExpression yearSuffix(R"(\s*\(\d{4}\)$)");
    return QString(title).remove(yearSuffix).trimmed();
}

/// \brief Extract a trailing 4-digit year from the query and return the query without it.
/// \details Users often type "Inception 2010". OMDb requires the year as a separate &y= parameter.
///          Returns {query, year} where year is 0 if no year was found.
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

OmdbMovieSearchJob::OmdbMovieSearchJob(OmdbApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void OmdbMovieSearchJob::doStart()
{
    if (ImdbId::isValidFormat(config().query)) {
        searchViaImdbId();
    } else {
        searchViaQuery();
    }
}

void OmdbMovieSearchJob::searchViaImdbId()
{
    // OMDb can look up a movie directly by IMDB ID
    m_api.loadMovie(ImdbId(config().query), [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }

        QJsonObject obj = json.object();
        MovieSearchJob::Result result;
        result.title = stripYearSuffix(obj.value("Title").toString());
        result.released = QDate::fromString(obj.value("Year").toString(), "yyyy");
        result.identifier = MovieIdentifier(obj.value("imdbID").toString());
        m_results.push_back(std::move(result));

        emitFinished();
    });
}

void OmdbMovieSearchJob::searchViaQuery()
{
    const auto [query, year] = extractYear(config().query);
    m_api.searchForMovie(query, year, 1, [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            // "Movie not found!" is a normal OMDb response for no results
            if (error.message == "Movie not found!") {
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

void OmdbMovieSearchJob::parseSearch(const QJsonDocument& json)
{
    QJsonArray results = json.object().value("Search").toArray();
    for (const QJsonValue& val : results) {
        QJsonObject obj = val.toObject();
        MovieSearchJob::Result result;
        result.title = stripYearSuffix(obj.value("Title").toString());
        result.released = QDate::fromString(obj.value("Year").toString(), "yyyy");
        result.identifier = MovieIdentifier(obj.value("imdbID").toString());
        m_results.push_back(std::move(result));
    }
}

} // namespace scraper
} // namespace mediaelch
