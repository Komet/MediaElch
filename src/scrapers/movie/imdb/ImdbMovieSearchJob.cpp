#include "scrapers/movie/imdb/ImdbMovieSearchJob.h"

#include "log/Log.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/imdb/ImdbSearchPage.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

ImdbMovieSearchJob::ImdbMovieSearchJob(ImdbApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void ImdbMovieSearchJob::doStart()
{
    if (ImdbId::isValidFormat(config().query)) {
        searchViaImdbId();
    } else {
        searchViaQuery();
    }
}

void ImdbMovieSearchJob::searchViaImdbId()
{
    MediaElch_Debug_Ensures(ImdbId::isValidFormat(config().query));

    m_api.loadTitleViaGraphQL(ImdbId(config().query), [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
        } else {
            parseGraphQLResult(data);
        }
        emitFinished();
    });
}

void ImdbMovieSearchJob::searchViaQuery()
{
    MediaElch_Debug_Ensures(!ImdbId::isValidFormat(config().query));

    m_api.suggestSearch(config().query, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
        } else {
            parseSuggestResults(data);
        }
        emitFinished();
    });
}

void ImdbMovieSearchJob::parseSuggestResults(const QString& json)
{
    // Movie types: movie, tvMovie, short, video, tvShort
    const QStringList movieTypes{"movie", "tvMovie", "short", "video", "tvShort"};
    auto results = ImdbSearchPage::parseSuggestResponse(json, movieTypes);
    for (const auto& result : results) {
        m_results << MovieSearchJob::Result{result.title, result.released, MovieIdentifier{result.identifier}};
    }
}

void ImdbMovieSearchJob::parseGraphQLResult(const QString& json)
{
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(generic) << "[ImdbMovieSearchJob] JSON parse error:" << parseError.errorString();
        return;
    }

    const QJsonObject title = doc.object().value("data").toObject().value("title").toObject();
    if (title.isEmpty()) {
        return;
    }

    MovieSearchJob::Result result;
    result.identifier = MovieIdentifier(config().query);
    result.title = title.value("titleText").toObject().value("text").toString();

    const QJsonObject releaseDate = title.value("releaseDate").toObject();
    const int year = releaseDate.value("year").toInt(0);
    if (year > 0) {
        const int month = releaseDate.value("month").toInt(1);
        const int day = releaseDate.value("day").toInt(1);
        result.released = QDate(year, month, day);
    }

    if (!result.title.isEmpty()) {
        m_results << result;
    }
}

} // namespace scraper
} // namespace mediaelch
