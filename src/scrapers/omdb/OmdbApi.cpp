#include "scrapers/omdb/OmdbApi.h"

#include "network/NetworkRequest.h"
#include "utils/Meta.h"

#include <QJsonObject>
#include <QTimer>
#include <QUrlQuery>

namespace mediaelch {
namespace scraper {

OmdbApi::OmdbApi(QObject* parent) : QObject(parent)
{
}

void OmdbApi::initialize()
{
    // no-op: OMDb requires no initialization beyond having an API key
}

bool OmdbApi::isInitialized() const
{
    return !m_apiKey.isEmpty();
}

void OmdbApi::setApiKey(const QString& apiKey)
{
    m_apiKey = apiKey;
}

void OmdbApi::sendGetRequest(const QUrl& url, OmdbApi::ApiCallback callback)
{
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);

    if (m_network.cache().hasValidElement(request)) {
        QTimer::singleShot(0, this, [cb = std::move(callback), element = m_network.cache().getElement(request)]() {
            cb(QJsonDocument::fromJson(element.toUtf8()), {});
        });
        return;
    }

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), request, this]() {
        auto dls = makeDeleteLaterScope(reply);

        if (reply->error() == QNetworkReply::NoError) {
            QString data = QString::fromUtf8(reply->readAll());
            m_network.cache().addElement(request, data);

            QJsonDocument json = QJsonDocument::fromJson(data.toUtf8());
            if (json.isNull()) {
                ScraperError error;
                error.error = ScraperError::Type::NetworkError;
                error.message = tr("OMDb API returned invalid JSON");
                cb({}, error);
                return;
            }

            // OMDb returns {"Response":"False","Error":"..."} on errors
            QJsonObject obj = json.object();
            if (obj.value("Response").toString() == "False") {
                ScraperError error;
                error.error = ScraperError::Type::ApiError;
                error.message = obj.value("Error").toString();
                cb({}, error);
                return;
            }

            cb(json, {});

        } else {
            ScraperError error;
            error.error = ScraperError::Type::NetworkError;
            error.message = reply->errorString();
            cb({}, error);
        }
    });
}

void OmdbApi::searchForMovie(const QString& query, int year, int page, ApiCallback callback)
{
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("s", query);
    urlQuery.addQueryItem("type", "movie");
    if (year > 0) {
        urlQuery.addQueryItem("y", QString::number(year));
    }
    if (page > 1) {
        urlQuery.addQueryItem("page", QString::number(page));
    }
    sendGetRequest(makeApiUrl(urlQuery), std::move(callback));
}

void OmdbApi::searchForShow(const QString& query, int year, int page, ApiCallback callback)
{
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("s", query);
    urlQuery.addQueryItem("type", "series");
    if (year > 0) {
        urlQuery.addQueryItem("y", QString::number(year));
    }
    if (page > 1) {
        urlQuery.addQueryItem("page", QString::number(page));
    }
    sendGetRequest(makeApiUrl(urlQuery), std::move(callback));
}

void OmdbApi::loadMovie(const ImdbId& imdbId, ApiCallback callback)
{
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("i", imdbId.toString());
    urlQuery.addQueryItem("plot", "full");
    sendGetRequest(makeApiUrl(urlQuery), std::move(callback));
}

void OmdbApi::loadShow(const ImdbId& imdbId, ApiCallback callback)
{
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("i", imdbId.toString());
    urlQuery.addQueryItem("plot", "full");
    sendGetRequest(makeApiUrl(urlQuery), std::move(callback));
}

void OmdbApi::loadSeason(const ImdbId& showId, int season, ApiCallback callback)
{
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("i", showId.toString());
    urlQuery.addQueryItem("Season", QString::number(season));
    sendGetRequest(makeApiUrl(urlQuery), std::move(callback));
}

void OmdbApi::loadEpisode(const ImdbId& episodeId, ApiCallback callback)
{
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("i", episodeId.toString());
    urlQuery.addQueryItem("plot", "full");
    sendGetRequest(makeApiUrl(urlQuery), std::move(callback));
}

QUrl OmdbApi::makeApiUrl(const QUrlQuery& query) const
{
    QUrlQuery fullQuery(query);
    fullQuery.addQueryItem("apikey", m_apiKey);

    QUrl url("https://www.omdbapi.com/");
    url.setQuery(fullQuery);
    return url;
}

} // namespace scraper
} // namespace mediaelch
