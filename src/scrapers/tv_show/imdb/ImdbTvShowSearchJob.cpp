#include "scrapers/tv_show/imdb/ImdbTvShowSearchJob.h"

#include "log/Log.h"
#include "scrapers/imdb/ImdbSearchPage.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

ImdbTvShowSearchJob::ImdbTvShowSearchJob(ImdbApi& api, ShowSearchJob::Config _config, QObject* parent) :
    ShowSearchJob(_config, parent), m_api{api}
{
}

void ImdbTvShowSearchJob::doStart()
{
    if (ImdbId::isValidFormat(config().query)) {
        searchViaImdbId();
    } else {
        searchViaQuery();
    }
}

void ImdbTvShowSearchJob::searchViaImdbId()
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

void ImdbTvShowSearchJob::searchViaQuery()
{
    MediaElch_Debug_Ensures(!ImdbId::isValidFormat(config().query));

    m_api.suggestSearch(config().query, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
        } else if (data.isEmpty()) {
            ScraperError emptyError;
            emptyError.error = ScraperError::Type::NetworkError;
            emptyError.message = tr("Loaded IMDb suggest response is empty. Cannot scrape requested TV show.");
            setScraperError(emptyError);
        } else {
            parseSuggestResults(data);
        }
        emitFinished();
    });
}

void ImdbTvShowSearchJob::parseSuggestResults(const QString& json)
{
    const QStringList tvTypes{"tvSeries", "tvMiniSeries"};
    auto results = ImdbSearchPage::parseSuggestResponse(json, tvTypes);
    for (const auto& result : results) {
        m_results << ShowSearchJob::Result{result.title, result.released, ShowIdentifier{result.identifier}};
    }
}

void ImdbTvShowSearchJob::parseGraphQLResult(const QString& json)
{
    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCWarning(generic) << "[ImdbTvShowSearchJob] JSON parse error:" << parseError.errorString();
        return;
    }

    const QJsonObject title = doc.object().value("data").toObject().value("title").toObject();
    if (title.isEmpty()) {
        return;
    }

    ShowSearchJob::Result result;
    result.identifier = ShowIdentifier(config().query);
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
