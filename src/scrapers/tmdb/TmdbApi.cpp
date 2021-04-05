#include "TmdbApi.h"

#include "Version.h"
#include "data/ImdbId.h"
#include "globals/Meta.h"
#include "log/Log.h"
#include "network/HttpStatusCodes.h"
#include "tv_shows/TvDbId.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace mediaelch {
namespace scraper {

TmdbApi::TmdbApi(QObject* parent) : QObject(parent)
{
}

void TmdbApi::initialize()
{
    QUrl url(TmdbApi::makeApiUrl("/configuration", Locale::English, {}));
    QNetworkRequest request = network::jsonRequestWithDefaults(url);
    QNetworkReply* const reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data{"{}"};
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());
            m_isInitialized = true;
            m_config = TmdbApiConfiguration::from(QJsonDocument::fromJson(data.toUtf8()));

        } else {
            qCWarning(generic) << "[TmdbApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
            m_isInitialized = false;
        }

        emit initialized(m_isInitialized);
    });
}

bool TmdbApi::isInitialized() const
{
    return m_isInitialized;
}

const TmdbApiConfiguration& TmdbApi::config() const
{
    return m_config;
}

void TmdbApi::sendGetRequest(const Locale& locale, const QUrl& url, TmdbApi::ApiCallback callback)
{
    if (m_cache.hasValidElement(url, locale)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, [cb = std::move(callback), element = m_cache.getElement(url, locale)]() {
            // should not result in a parse error because the cache element is
            // only stored if no error occured at all.
            cb(QJsonDocument::fromJson(element.toUtf8()), {});
        });
        return;
    }

    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);
    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), locale, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qCWarning(generic) << "[TmdbApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        QJsonParseError parseError{};
        QJsonDocument json;
        if (!data.isEmpty()) {
            json = QJsonDocument::fromJson(data.toUtf8(), &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                m_cache.addElement(reply->url(), locale, data);
            }
        }

        ScraperError error = makeScraperError(data, *reply, parseError);
        cb(json, error);
    });
}

void TmdbApi::searchForShow(const Locale& locale,
    const QString& query,
    bool includeAdult,
    TmdbApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowSearchUrl(query, locale, includeAdult), std::move(callback));
}

void TmdbApi::loadShowInfos(const Locale& locale, const TmdbId& id, TmdbApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowUrl(id, locale), callback);
}

void TmdbApi::loadMinimalInfos(const Locale& locale, const TmdbId& id, TmdbApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowUrl(id, locale, true), callback);
}

void TmdbApi::loadEpisode(const Locale& locale,
    const TmdbId& showId,
    SeasonNumber season,
    EpisodeNumber episode,
    ApiCallback callback)
{
    sendGetRequest(locale, getEpisodeUrl(showId, season, episode, locale), callback);
}

void TmdbApi::loadSeason(const Locale& locale,
    const TmdbId& showId,
    SeasonNumber season,
    SeasonOrder order,
    ApiCallback callback)
{
    Q_UNUSED(order);
    sendGetRequest(locale, getSeasonUrl(showId, season, locale), callback);
}

void TmdbApi::searchForConcert(const Locale& locale, const QString& query, TmdbApi::ApiCallback callback)
{
    sendGetRequest(locale, getMovieSearchUrl(query, locale, false, {}), std::move(callback));
}

QUrl TmdbApi::makeApiUrl(const QString& suffix, const Locale& locale, QUrlQuery query) const
{
    query.addQueryItem("api_key", TmdbApi::apiKey());
    query.addQueryItem("language", locale.toString('-'));

    return QStringLiteral("https://api.themoviedb.org/3%1?%2").arg(suffix, query.toString());
}

QUrl TmdbApi::makeImageUrl(const QString& suffix) const
{
    // TODO: Use image sizes
    return QUrl(config().imageSecureBaseUrl + "original" + suffix);
}

QUrl TmdbApi::getShowSearchUrl(const QString& searchStr, const Locale& locale, bool includeAdult) const
{
    QUrlQuery queries;
    // Special handling of certain ID types. TheMovieDb supports other IDs and not only
    // their TMDb IDs.
    if (TmdbId::isValidFormat(searchStr)) {
        return makeApiUrl(QStringLiteral("/tv/") + searchStr, locale, queries);
    }
    if (ImdbId::isValidFormat(searchStr)) {
        queries.addQueryItem("external_source", "imdb_id");
        return makeApiUrl(QStringLiteral("/find/") + searchStr, locale, queries);
    }
    if (TvDbId::isValidPrefixedFormat(searchStr)) {
        queries.addQueryItem("external_source", "tvdb_id");
        return makeApiUrl(QStringLiteral("/find/") + searchStr, locale, queries);
    }

    queries.addQueryItem("page", "1"); // Only query first page as of now.
    queries.addQueryItem("query", searchStr);
    queries.addQueryItem("include_adult", includeAdult ? "true" : "false");
    return makeApiUrl("/search/tv", locale, queries);
}

QUrl TmdbApi::getShowUrl(const TmdbId& id, const Locale& locale, bool onlyBasicDetails) const
{
    QUrlQuery queries;
    if (!onlyBasicDetails) {
        // Instead of multiple HTTP requests, use just one for everything.
        queries.addQueryItem("append_to_response", "content_ratings,keywords,external_ids,images,credits");
    }
    return makeApiUrl(QStringLiteral("/tv/") + id.toString(), locale, queries);
}

QUrl TmdbApi::getEpisodeUrl(const TmdbId& showId,
    SeasonNumber season,
    EpisodeNumber episode,
    const Locale& locale) const
{
    QString url =
        QStringLiteral("/tv/%1/season/%2/episode/%3").arg(showId.toString(), season.toString(), episode.toString());
    QUrlQuery queries;
    // Instead of multiple HTTP requests, use just one for everything.
    queries.addQueryItem("append_to_response", "external_ids,credits");
    return makeApiUrl(url, locale, queries);
}

QUrl TmdbApi::getSeasonUrl(const TmdbId& showId, SeasonNumber season, const Locale& locale) const
{
    QString url = QStringLiteral("/tv/%1/season/%2").arg(showId.toString(), season.toString());
    QUrlQuery queries;
    // Instead of multiple HTTP requests, use just one for everything.
    queries.addQueryItem("append_to_response", "external_ids,credits");
    return makeApiUrl(url, locale, queries);
}

QUrl TmdbApi::getMovieSearchUrl(const QString& searchStr,
    const Locale& locale,
    bool includeAdult,
    const UrlParameterMap& parameters) const
{
    QUrlQuery queries;
    // Special handling of certain ID types. TheMovieDb supports other IDs and not only
    // their TMDb IDs.
    if (TmdbId::isValidFormat(searchStr)) {
        return makeApiUrl(QStringLiteral("/movie/") + searchStr, locale, queries);
    }
    if (ImdbId::isValidFormat(searchStr)) {
        queries.addQueryItem("external_source", "imdb_id");
        return makeApiUrl(QStringLiteral("/find/") + searchStr, locale, queries);
    }
    if (TvDbId::isValidPrefixedFormat(searchStr)) {
        queries.addQueryItem("external_source", "tvdb_id");
        return makeApiUrl(QStringLiteral("/find/") + searchStr, locale, queries);
    }

    queries.addQueryItem("page", "1"); // Only query first page as of now.
    queries.addQueryItem("query", searchStr);
    queries.addQueryItem("include_adult", includeAdult ? "true" : "false");

    const auto keys = parameters.keys();
    for (const auto& key : keys) {
        queries.addQueryItem(apiUrlParameterString(key), parameters.value(key));
    }

    return makeApiUrl("/search/movie", locale, queries);
}

QString TmdbApi::apiUrlParameterString(TmdbApi::ApiUrlParameter parameter) const
{
    switch (parameter) {
    case ApiUrlParameter::YEAR: return QStringLiteral("year");
    case ApiUrlParameter::PAGE: return QStringLiteral("page");
    case ApiUrlParameter::INCLUDE_ADULT: return QStringLiteral("include_adult");
    }
    qCCritical(generic) << "[TMDb] ApiUrlParameter: Unhandled enum case.";
    return QStringLiteral("unknown");
}

/// \brief Get the movie URL for TMDb. Adds the API key.
QUrl TmdbApi::getMovieUrl(QString movieId,
    const Locale& locale,
    ApiMovieDetails type,
    const UrlParameterMap& parameters) const
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
    queries.addQueryItem("api_key", TmdbApi::apiKey());
    queries.addQueryItem("language", locale.toString('-'));

    if (type == ApiMovieDetails::IMAGES) {
        queries.addQueryItem("include_image_language", "en,null," + locale.language());
    }

    const auto keys = parameters.keys();
    for (const auto& key : keys) {
        queries.addQueryItem(apiUrlParameterString(key), parameters.value(key));
    }

    return QUrl{url.append(queries.toString())};
}

/// \brief Get the collection URL for TMDb. Adds the API key.
QUrl TmdbApi::getCollectionUrl(QString collectionId, const Locale& locale) const
{
    auto url = QStringLiteral("https://api.themoviedb.org/3/collection/%1?").arg(collectionId);

    QUrlQuery queries;
    queries.addQueryItem("api_key", TmdbApi::apiKey());
    queries.addQueryItem("language", locale.toString('-'));

    return QUrl{url.append(queries.toString())};
}

QString TmdbApi::apiKey()
{
    // TheMovieTv API v3 key for MediaElch
    return QStringLiteral("5d832bdf69dcb884922381ab01548d5b");
}

TmdbApiConfiguration TmdbApiConfiguration::from(QJsonDocument doc)
{
    QJsonObject obj = doc.object();
    QJsonObject images = obj["images"].toObject();

    const auto assignStringUrl = [](QString& target, const QString& source) {
        if (source.isEmpty()) {
            return;
        }
        target = source;
        if (target.at(target.length() - 1) != '/') {
            // Ensure that it ends with a slash
            target += '/';
        }
    };

    TmdbApiConfiguration config;
    assignStringUrl(config.imageBaseUrl, images["base_url"].toString());
    assignStringUrl(config.imageSecureBaseUrl, images["secure_base_url"].toString());

    const auto assignStringArray = [](QStringList& target, const QJsonArray& array) {
        for (const QJsonValue& val : array) {
            target << val.toString();
        }
    };

    assignStringArray(config.backdropSizes, images["backdrop_sizes"].toArray());
    assignStringArray(config.logoSizes, images["logo_sizes"].toArray());
    assignStringArray(config.posterSizes, images["poster_sizes"].toArray());
    assignStringArray(config.profileSizes, images["profile_sizes"].toArray());
    assignStringArray(config.stillSizes, images["still_sizes"].toArray());

    return config;
}

} // namespace scraper
} // namespace mediaelch
