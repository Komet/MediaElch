#include "ImdbApi.h"

#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "scrapers/imdb/ImdbGraphQLQueries.h"
#include "utils/Meta.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
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

QUrl ImdbApi::makeFullUrl(const QString& suffix)
{
    MediaElch_Debug_Expects(suffix.startsWith('/'));
    return {"https://www.imdb.com" + suffix};
}

QUrl ImdbApi::makeFullAssetUrl(const QString& suffix)
{
    return {"https://www.imdb.com" + suffix};
}

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
