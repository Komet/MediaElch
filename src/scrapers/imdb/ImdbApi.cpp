#include "ImdbApi.h"

#include "Version.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/imdb/ImdbGraphQLQueries.h"
#include "utils/Meta.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookie>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace mediaelch {
namespace scraper {

ImdbApi::ImdbApi(QObject* parent) : QObject(parent)
{
}

void ImdbApi::initialize()
{
    // no-op
}

bool ImdbApi::isInitialized() const
{
    return true;
}

void ImdbApi::sendGetRequest(const Locale& locale, const QUrl& url, ImdbApi::ApiCallback callback)
{
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    addHeadersToRequest(locale, request);
    // The IMDB does not accept requests with the MediaElch HTTP request user agent
    mediaelch::network::useFirefoxUserAgent(request);

    if (m_network.cache().hasValidElement(request)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, this, [cb = std::move(callback), element = m_network.cache().getElement(request)]() { //
            cb(element, {});
        });
        return;
    }

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), request, locale, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString html;
        if (reply->error() == QNetworkReply::NoError) {
            html = QString::fromUtf8(reply->readAll());

            if (!html.isEmpty()) {
                m_network.cache().addElement(request, html);
            }
        } else {
            qCWarning(generic) << "[ImdbTv][Api] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        ScraperError error = makeScraperError(html, *reply, {});
        cb(html, error);
    });
}

void ImdbApi::searchForShow(const Locale& locale, const QString& query, ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, makeShowSearchUrl(query), std::move(callback));
}

void ImdbApi::searchForMovie(const Locale& locale,
    const QString& query,
    bool includeAdult,
    ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, makeMovieSearchUrl(query, includeAdult), std::move(callback));
}

void mediaelch::scraper::ImdbApi::loadTitle(const Locale& locale,
    const ImdbId& movieId,
    PageKind page,
    ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, makeTitleUrl(movieId, page), callback);
}

void ImdbApi::loadDefaultEpisodesPage(const Locale& locale, const ImdbId& showId, ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, makeDefaultEpisodesUrl(showId), callback);
}

void ImdbApi::loadSeason(const Locale& locale, const ImdbId& showId, SeasonNumber season, ImdbApi::ApiCallback callback)
{
    sendGetRequest(locale, makeSeasonUrl(showId, season), callback);
}

void ImdbApi::addHeadersToRequest(const Locale& locale, QNetworkRequest& request)
{
    request.setRawHeader("Accept-Language", locale.toString('-').toLocal8Bit());

    QNetworkCookie languageCookie("lc-main", locale.toString('_').toLocal8Bit());
    QList<QNetworkCookie> cookies{{languageCookie}};
    request.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue(cookies));
}

QUrl ImdbApi::makeTitleUrl(const ImdbId& id, PageKind page) const
{
    const QString pageStr = [page]() {
        switch (page) {
        case PageKind::Main: return "";
        case PageKind::Reference: return "reference";
        case PageKind::PlotSummary: return "plotsummary";
        case PageKind::ReleaseInfo: return "releaseinfo";
        case PageKind::Keywords: return "keywords";
        case PageKind::Episodes: return "episodes";
        }
        qCCritical(generic, "[ImdbApi] Unhandled page key!");
        return "";
    }();
    return makeFullUrl(QStringLiteral("/title/%1/%2").arg(id.toString(), pageStr));
}

QUrl ImdbApi::makeMovieSearchUrl(const QString& searchStr, bool includeAdult) const
{
    // e.g. https://www.imdb.com/de/search/title/?title=finding%20dori&title_type=feature,tv_movie,short,video,tv_short
    QUrlQuery queries;
    if (includeAdult) {
        queries.addQueryItem("adult", "include");
    }
    queries.addQueryItem("title", searchStr);
    queries.addQueryItem("title_type", "feature,tv_movie,short,video,tv_short"); // Movie categories
    queries.addQueryItem("view", "simple");
    queries.addQueryItem("count", "100");
    return makeFullUrl("/search/title/?" + queries.toString());
}

QUrl ImdbApi::makeFullUrl(const QString& suffix)
{
    MediaElch_Debug_Expects(suffix.startsWith('/'));
    return {"https://www.imdb.com" + suffix};
}

QUrl ImdbApi::makeFullAssetUrl(const QString& suffix)
{
    return {"https://www.imdb.com" + suffix};
}

QUrl ImdbApi::makeShowSearchUrl(const QString& searchStr) const
{
    if (ImdbId::isValidFormat(searchStr)) {
        return makeFullUrl(QStringLiteral("/title/") + searchStr + '/');
    }

    // e.g. https://www.imdb.com/search/title/?title=Family%20Guy&title_type=tv_series,tv_miniseries&view=simple
    // An alternative (if it breaks in the future) would be
    // e.g. https://www.imdb.com/find/?q=scrubs&s=tt&ttype=tv&ref_=fn_tv
    QUrlQuery queries;
    queries.addQueryItem("title", searchStr);
    queries.addQueryItem("title_type", "tv_series,tv_miniseries");
    queries.addQueryItem("view", "simple");
    queries.addQueryItem("count", "100");
    return makeFullUrl("/search/title/?" + queries.toString());
}

QUrl ImdbApi::makeSeasonUrl(const ImdbId& showId, SeasonNumber season) const
{
    // e.g. https://www.imdb.com/title/tt0096697/episodes/?season=10
    QUrlQuery queries;
    queries.addQueryItem("season", season.toString());
    return makeFullUrl(QStringLiteral("/title/") + showId.toString() + //
                       QStringLiteral("/episodes?") + queries.toString());
}

QUrl ImdbApi::makeDefaultEpisodesUrl(const ImdbId& showId) const
{
    return makeTitleUrl(showId, PageKind::Episodes);
}

// --- New GraphQL + Suggest API methods ---

QUrl ImdbApi::makeSuggestUrl(const QString& query)
{
    // The Suggest API uses the first character of the query as a path segment.
    // e.g. "inception" -> https://v3.sg.media-imdb.com/suggestion/x/inception.json
    // The path letter doesn't seem to matter, so we use 'x' for simplicity.
    QString normalized = query.toLower().trimmed();
    normalized = QString(QUrl::toPercentEncoding(normalized));
    return QUrl(QStringLiteral("https://v3.sg.media-imdb.com/suggestion/x/%1.json").arg(normalized));
}

QUrl ImdbApi::makeGraphQLUrl()
{
    return QUrl(QStringLiteral("https://graphql.imdb.com/"));
}

void ImdbApi::suggestSearch(const QString& query, ImdbApi::ApiCallback callback)
{
    const QUrl url = makeSuggestUrl(query);
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    mediaelch::network::useFirefoxUserAgent(request);

    if (m_network.cache().hasValidElement(request)) {
        QTimer::singleShot(0, this, [cb = std::move(callback), element = m_network.cache().getElement(request)]() {
            cb(element, {});
        });
        return;
    }

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), request, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());
            if (!data.isEmpty()) {
                m_network.cache().addElement(request, data);
            }
        } else {
            qCWarning(generic) << "[ImdbApi] Suggest API Network Error:" << reply->errorString() << "for URL"
                               << reply->url();
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

void ImdbApi::sendGraphQLRequest(const QString& query,
    const QJsonObject& variables,
    ImdbApi::ApiCallback callback)
{
    QJsonObject body;
    body["query"] = query;
    if (!variables.isEmpty()) {
        body["variables"] = variables;
    }
    const QByteArray postData = QJsonDocument(body).toJson(QJsonDocument::Compact);

    // The WebsiteCache keys by URL only (QMap<QUrl, CacheElement>).
    // Since all GraphQL requests go to the same URL, we append a hash of the POST body
    // as a query parameter to create unique cache keys.
    QUrl cacheUrl = makeGraphQLUrl();
    QUrlQuery cacheQuery;
    cacheQuery.addQueryItem("_body",
        QString::fromLatin1(QCryptographicHash::hash(postData, QCryptographicHash::Md5).toHex()));
    cacheUrl.setQuery(cacheQuery);

    QNetworkRequest cacheRequest = mediaelch::network::jsonRequestWithDefaults(cacheUrl);

    if (m_network.cache().hasValidElement(cacheRequest)) {
        QTimer::singleShot(
            0, this, [cb = std::move(callback), element = m_network.cache().getElement(cacheRequest)]() {
                cb(element, {});
            });
        return;
    }

    // The actual request goes to the real URL (without the cache query parameter)
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(makeGraphQLUrl());
    mediaelch::network::useFirefoxUserAgent(request);

    QNetworkReply* reply = m_network.postWithWatcher(request, postData);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), cacheRequest, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());
            if (!data.isEmpty()) {
                m_network.cache().addElement(cacheRequest, data);
            }
        } else {
            qCWarning(generic) << "[ImdbApi] GraphQL Network Error:" << reply->errorString() << "for URL"
                               << reply->url();
        }

        ScraperError error = makeScraperError(data, *reply, {});
        cb(data, error);
    });
}

void ImdbApi::loadTitleViaGraphQL(const ImdbId& id, ImdbApi::ApiCallback callback)
{
    QJsonObject variables;
    variables["id"] = id.toString();
    sendGraphQLRequest(ImdbGraphQLQueries::TITLE_DETAILS, variables, std::move(callback));
}

void ImdbApi::loadEpisodesViaGraphQL(const ImdbId& showId, int limit, ImdbApi::ApiCallback callback)
{
    QJsonObject variables;
    variables["id"] = showId.toString();
    variables["first"] = limit;
    sendGraphQLRequest(ImdbGraphQLQueries::SEASON_EPISODES, variables, std::move(callback));
}

} // namespace scraper
} // namespace mediaelch
