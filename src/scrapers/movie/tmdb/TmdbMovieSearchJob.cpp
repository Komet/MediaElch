#include "scrapers/movie/tmdb/TmdbMovieSearchJob.h"

#include "scrapers/tmdb/TmdbApi.h"

#include <QJsonArray>

namespace mediaelch {
namespace scraper {

TmdbMovieSearchJob::TmdbMovieSearchJob(TmdbApi& api, MovieSearchJob::Config _config, QObject* parent) :
    MovieSearchJob(_config, parent), m_api{api}
{
}

void TmdbMovieSearchJob::execute()
{
    QString searchStr = QString(config().query).replace("-", " ").trimmed();

    if (searchStr.isEmpty()) {
        // searching without a query results in a network error
        emit sigFinished(this);
        return;
    }

    QString searchTitle;
    QString searchYear;
    QUrl url;
    QString includeAdult = config().includeAdult ? "true" : "false";

    const bool isSearchByImdbId = QRegExp("^tt\\d+$").exactMatch(searchStr);
    const bool isSearchByTmdbId = QRegExp("^id\\d+$").exactMatch(searchStr);

    if (isSearchByImdbId) {
        QUrl newUrl(m_api.getMovieUrl(searchStr,
            config().locale,
            TmdbApi::ApiMovieDetails::INFOS,
            TmdbApi::UrlParameterMap{{TmdbApi::ApiUrlParameter::INCLUDE_ADULT, includeAdult}}));
        url.swap(newUrl);

    } else if (isSearchByTmdbId) {
        QUrl newUrl(m_api.getMovieUrl(searchStr.mid(2),
            config().locale,
            TmdbApi::ApiMovieDetails::INFOS,
            TmdbApi::UrlParameterMap{{TmdbApi::ApiUrlParameter::INCLUDE_ADULT, includeAdult}}));
        url.swap(newUrl);

    } else {
        QUrl newUrl(m_api.getMovieSearchUrl(searchStr,
            config().locale,
            config().includeAdult,
            TmdbApi::UrlParameterMap{{TmdbApi::ApiUrlParameter::INCLUDE_ADULT, includeAdult}}));
        url.swap(newUrl);
        QVector<QRegExp> rxYears;
        rxYears << QRegExp(R"(^(.*) \((\d{4})\)$)") << QRegExp("^(.*) (\\d{4})$") << QRegExp("^(.*) - (\\d{4})$");
        for (QRegExp rxYear : rxYears) {
            rxYear.setMinimal(true);
            if (rxYear.exactMatch(searchStr)) {
                searchTitle = rxYear.cap(1);
                searchYear = rxYear.cap(2);
                QUrl newSearchUrl = m_api.getMovieSearchUrl(searchTitle,
                    config().locale,
                    config().includeAdult,
                    TmdbApi::UrlParameterMap{{TmdbApi::ApiUrlParameter::INCLUDE_ADULT, includeAdult},
                        {TmdbApi::ApiUrlParameter::YEAR, searchYear}});
                url.swap(newSearchUrl);
                break;
            }
        }
    }

    m_api.sendGetRequest(config().locale, url, [this](QJsonDocument json, ScraperError error) {
        if (error.hasError()) {
            m_error = error;
            emit sigFinished(this);
            return;
        }

        int nextPage = -1;
        parseSearch(json.object(), &nextPage);

        if (nextPage == -1) {
            // no more pages to look for
            emit sigFinished(this);
            return;
        }

        m_currentSearchPage = nextPage;

        // TODO: Load next search page
        emit sigFinished(this);
    });
}

void TmdbMovieSearchJob::parseSearch(const QJsonObject& json, int* nextPage)
{
    // only get the first 3 pages
    if (m_currentSearchPage < json.value("total_pages").toInt() && m_currentSearchPage < 3) {
        *nextPage = m_currentSearchPage + 1;
    }

    if (json.value("results").isArray()) {
        const auto jsonResults = json.value("results").toArray();
        for (const auto& it : jsonResults) {
            const auto resultObj = it.toObject();
            if (resultObj.value("id").toInt() == 0) {
                continue;
            }
            MovieSearchJob::Result result;
            result.title = resultObj.value("title").toString();
            if (result.title.isEmpty()) {
                result.title = resultObj.value("original_title").toString();
            }
            result.identifier = MovieIdentifier(QString::number(resultObj.value("id").toInt()));
            result.released = QDate::fromString(resultObj.value("release_date").toString(), "yyyy-MM-dd");
            m_results.append(result);
        }

    } else if (json.value("id").toInt() > 0) {
        MovieSearchJob::Result result;
        result.title = json.value("title").toString();
        if (result.title.isEmpty()) {
            result.title = json.value("original_title").toString();
        }
        result.identifier = MovieIdentifier(QString::number(json.value("id").toInt()));
        result.released = QDate::fromString(json.value("release_date").toString(), "yyyy-MM-dd");
        m_results.append(result);
    }
}

} // namespace scraper
} // namespace mediaelch
