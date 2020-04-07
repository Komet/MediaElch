#include "scrapers/movie/tmdb/TmdbApi.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"

#include <QDebug>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QUrlQuery>

#include "Version.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

namespace mediaelch {
namespace scraper {

/// \brief Get a string representation of ApiUrlParameter
static QString apiUrlParameterString(TmdbApi::ApiUrlParameter parameter)
{
    switch (parameter) {
    case TmdbApi::ApiUrlParameter::Locale: return QStringLiteral("language");
    case TmdbApi::ApiUrlParameter::YEAR: return QStringLiteral("year");
    case TmdbApi::ApiUrlParameter::PAGE: return QStringLiteral("page");
    case TmdbApi::ApiUrlParameter::INCLUDE_ADULT: return QStringLiteral("include_adult");
    }
    qCritical() << "[TmdbApi] ApiUrlParameter: Unhandled enum case.";
    return QStringLiteral("unknown");
}

/**
 * \brief Get the movie search URL for TmdbMovie. Adds the API key and language.
 * \param searchStr Search string. Will be percent encoded.
 * \param parameters A QMap of URL parameters. The values will be percent encoded.
 */
QUrl TmdbApi::movieSearchUrl(const QString& searchStr, const UrlParameterMap& parameters) const
{
    auto url = QStringLiteral("https://api.themoviedb.org/3/search/movie?");

    QUrlQuery queries;
    queries.addQueryItem("api_key", key());
    queries.addQueryItem("query", searchStr);

    for (const auto& key : parameters.keys()) {
        queries.addQueryItem(apiUrlParameterString(key), parameters.value(key));
    }

    return QUrl{url.append(queries.toString())};
}

/// \brief Get the movie URL for TmdbMovie. Adds the API key.
QUrl TmdbApi::movieUrl(QString movieId, ApiMovieDetails type, const UrlParameterMap& parameters) const
{
    const auto typeStr = [type]() {
        switch (type) {
        case ApiMovieDetails::INFOS: return QString{};
        case ApiMovieDetails::IMAGES: return QStringLiteral("/images");
        case ApiMovieDetails::CASTS: return QStringLiteral("/casts");
        case ApiMovieDetails::TRAILERS: return QStringLiteral("/trailers");
        case ApiMovieDetails::RELEASES: return QStringLiteral("/releases");
        }
        return QString{};
    }();

    auto url =
        QStringLiteral("https://api.themoviedb.org/3/movie/%1%2?").arg(QUrl::toPercentEncoding(movieId), typeStr);
    QUrlQuery queries;
    queries.addQueryItem("api_key", key());

    if (type == ApiMovieDetails::IMAGES) {
        // Always load English images
        QString lang = "en,null";
        if (parameters.contains(TmdbApi::ApiUrlParameter::Locale)) {
            lang += "," + parameters[TmdbApi::ApiUrlParameter::Locale];
        }
        queries.addQueryItem("include_image_language", lang);
    }

    for (const auto& key : parameters.keys()) {
        queries.addQueryItem(apiUrlParameterString(key), parameters.value(key));
    }

    return QUrl{url.append(queries.toString())};
}

/// \brief Get the collection URL for TmdbMovie. Adds the API key.
QUrl TmdbApi::collectionUrl(QString collectionId) const
{
    auto url = QStringLiteral("https://api.themoviedb.org/3/collection/%1?").arg(collectionId);

    QUrlQuery queries;
    queries.addQueryItem("api_key", key());

    return QUrl{url.append(queries.toString())};
}

QNetworkReply* TmdbApi::sendGetRequest(const QUrl& url)
{
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, mediaelch::currentVersionIdentifier());

    QNetworkReply* const reply = m_qnam.get(request);
    new NetworkReplyWatcher(this, reply);
    return reply;
}

QVector<Locale> TmdbApi::supportedLanguages()
{
    // For officially supported languages, see:
    // https://developers.themoviedb.org/3/configuration/get-primary-translations
    return {{"ar"},
        {"bg"},
        {"zh-TW"},
        {"zh-CN"},
        {"hr"},
        {"cs"},
        {"da"},
        {"nl"},
        {"en"},
        {"en-US"},
        {"fi"},
        {"fr"},
        {"fr-CA"},
        {"de"},
        {"el"},
        {"he"},
        {"hu"},
        {"it"},
        {"ja"},
        {"ko"},
        {"no"},
        {"pl"},
        {"pt-BR"},
        {"pt-PT"},
        {"ru"},
        {"sl"},
        {"es"},
        {"es-MX"},
        {"sv"},
        {"tr"}};
}


} // namespace scraper
} // namespace mediaelch
