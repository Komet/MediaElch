#include "scrapers/movie/tmdb/TmdbMovieSearch.h"

#include "data/ImdbId.h"
#include "globals/Globals.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegularExpression>

namespace mediaelch {
namespace scraper {


void TmdbMovieSearchJob::execute()
{
    qDebug() << "[TmdbMovieSearchJob] Start movie search:" << query() << "for language" << config().locale.toString();

    QString searchStr = normalizedQuery();
    QString includeAdult = (config().includeAdtulResults) ? "true" : "false";

    const bool isSearchByImdbId = ImdbId::isValidFormat(searchStr);
    const bool isSearchByTmdbId = QRegularExpression("^id\\d+$").match(searchStr).hasMatch();

    auto urlParams = TmdbApi::UrlParameterMap{{TmdbApi::ApiUrlParameter::INCLUDE_ADULT, includeAdult},
        {TmdbApi::ApiUrlParameter::Locale, config().locale.toString()}};

    QUrl url;

    if (isSearchByImdbId) {
        url = m_api.movieUrl(searchStr, TmdbApi::ApiMovieDetails::INFOS, urlParams);

    } else if (isSearchByTmdbId) {
        QString id = searchStr.mid(2); // remove "id" prefix
        url = m_api.movieUrl(id, TmdbApi::ApiMovieDetails::INFOS, urlParams);

    } else {
        // "normal" search
        QPair<QString, QString> titleYear = MovieSearchJob::extractTitleAndYear(query());
        m_searchTitle = titleYear.first;
        m_searchYear = titleYear.second;

        if (!m_searchTitle.isEmpty() && !m_searchYear.isEmpty()) {
            searchStr = m_searchTitle;
            urlParams.insert(TmdbApi::ApiUrlParameter::YEAR, m_searchYear);
        }

        url = m_api.movieSearchUrl(searchStr, urlParams);
    }

    QNetworkReply* const reply = m_api.sendGetRequest(url);
    connect(reply, &QNetworkReply::finished, this, &TmdbMovieSearchJob::handleSearchResponse);
}


/// \brief Called when the search result was downloaded
///        Emits "searchDone" if there are no more pages in the result set
/// \see TmdbMovie::parseSearch
void TmdbMovieSearchJob::handleSearchResponse()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[TmdbMovieSearchJob] handleSearchResponse: nullptr reply | Please report this issue!";
        emit sigSearchError({ScraperSearchError::ErrorType::InternalError, tr("Internal Error: Please report!")});
        return;
    }

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[TmdbMovieSearchJob] Network Error" << reply->errorString();
        emit sigSearchError({ScraperSearchError::ErrorType::NetworkError, reply->errorString()});
        return;
    }

    const QString json = QString::fromUtf8(reply->readAll());
    int nextPage = -1;
    m_results.append(parseSearch(json, &nextPage));
    reply->deleteLater();

    if (nextPage == -1) {
        emit sigSearchSuccess(m_results);

    } else {
        QString nextPageStr{QString::number(nextPage)};
        const QUrl url = [&]() {
            if (m_searchTitle.isEmpty() || m_searchYear.isEmpty()) {
                return m_api.movieSearchUrl(
                    normalizedQuery(), TmdbApi::UrlParameterMap{{TmdbApi::ApiUrlParameter::PAGE, nextPageStr}});
            }
            return m_api.movieSearchUrl(m_searchTitle,
                TmdbApi::UrlParameterMap{
                    {TmdbApi::ApiUrlParameter::PAGE, nextPageStr}, {TmdbApi::ApiUrlParameter::YEAR, m_searchYear}});
        }();

        m_currentTmdbSearchPage = nextPage;

        QNetworkReply* const searchReply = m_api.sendGetRequest(url);
        connect(searchReply, &QNetworkReply::finished, this, &TmdbMovieSearchJob::handleSearchResponse);
    }
}

QString TmdbMovieSearchJob::normalizedQuery()
{
    return QString(query()).replace("-", " ");
}

QVector<MovieSearchJob::Result> TmdbMovieSearchJob::parseSearch(QString json, int* nextPage)
{
    QVector<MovieSearchJob::Result> results;

    QJsonParseError parseError{};
    const auto parsedJson = QJsonDocument::fromJson(json.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError) {
        // TODO: Notify
        qWarning() << "[TmdbMovieSearchJob] Error parsing search json" << parseError.errorString();
        return results;
    }

    // only get the first 3 pages
    if (m_currentTmdbSearchPage < parsedJson.value("total_pages").toInt() && m_currentTmdbSearchPage < m_maxPages) {
        *nextPage = m_currentTmdbSearchPage + 1;
    }

    auto parseSingleResult = [](auto& jsonObj) {
        MovieSearchJob::Result result;
        result.title = jsonObj.value("title").toString();
        if (result.title.isEmpty()) {
            result.title = jsonObj.value("original_title").toString();
        }
        result.identifier = QString::number(jsonObj.value("id").toInt());
        result.released = QDate::fromString(jsonObj.value("release_date").toString(), "yyyy-MM-dd");
        return result;
    };

    if (parsedJson.value("results").isArray()) {
        const auto jsonResults = parsedJson.value("results").toArray();

        for (const auto& it : jsonResults) {
            const auto resultObj = it.toObject();
            if (resultObj.value("id").toInt(0) > 0) {
                results.append(parseSingleResult(resultObj));
            }
        }

    } else if (parsedJson.value("id").toInt(0) > 0) {
        results.append(parseSingleResult(parsedJson));
    }

    return results;
}

} // namespace scraper
} // namespace mediaelch
