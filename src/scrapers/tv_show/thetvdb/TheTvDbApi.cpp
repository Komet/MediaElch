#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"

#include "Version.h"
#include "globals/JsonRequest.h"
#include "globals/Meta.h"
#include "network/NetworkRequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace mediaelch {
namespace scraper {

TheTvDbApi::TheTvDbApi(QObject* parent) : QObject(parent)
{
}

void TheTvDbApi::initialize()
{
    const QJsonObject body{{"apikey", "A0BB9A0F6762942B"}};
    auto* request = new mediaelch::JsonPostRequest(makeFullUrl("/login"), body, this);
    connect(request, &mediaelch::JsonPostRequest::sigResponse, this, [this, request](QJsonDocument& parsedJson) {
        qDebug() << "[TheTvDbApi] Received JSON web token";
        request->deleteLater();
        ApiToken token(parsedJson.object().value("token").toString());
        if (token.isValid()) {
            m_token = token;
        }
        emit initialized(token.isValid());
    });
}

bool TheTvDbApi::isInitialized() const
{
    return m_token.isValid();
}

void TheTvDbApi::sendGetRequest(const Locale& locale, const QUrl& url, TheTvDbApi::ApiCallback callback)
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
    addHeadersToRequest(locale, request);

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), locale, this]() {
        auto dls = makeDeleteLaterScope(reply);

        QString data;
        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qWarning() << "[TheTvDbApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
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

void TheTvDbApi::searchForShow(const Locale& locale, const QString& query, TheTvDbApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowSearchUrl(query), std::move(callback));
}

void TheTvDbApi::loadShowInfos(const Locale& locale, const TvDbId& id, TheTvDbApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowUrl(ApiShowDetails::INFOS, id), callback);
}

void TheTvDbApi::loadShowActors(const Locale& locale, const TvDbId& id, TheTvDbApi::ApiCallback callback)
{
    sendGetRequest(locale, getShowUrl(ApiShowDetails::ACTORS, id), callback);
}

void TheTvDbApi::loadImageUrls(const Locale& locale,
    const TvDbId& id,
    ShowScraperInfo imageType,
    TheTvDbApi::ApiCallback callback)
{
    sendGetRequest(locale, getImagesUrl(imageType, id), callback);
}

void TheTvDbApi::loadSeason(const Locale& locale,
    const TvDbId& id,
    SeasonNumber season,
    SeasonOrder order,
    TheTvDbApi::ApiCallback callback)
{
    sendGetRequest(locale, getSeasonUrl(id, season, order), callback);
}

void TheTvDbApi::loadSeasonsPage(const Locale& locale,
    const TvDbId& id,
    const QSet<SeasonNumber>& seasons,
    SeasonOrder order,
    ApiPage page,
    ApiCallback callback)
{
    Q_UNUSED(seasons);
    Q_UNUSED(order);
    // TODO: Only load requested seasons.
    sendGetRequest(locale, getEpisodesUrl(id, page), callback);
}

void TheTvDbApi::loadAllSeasonsPage(const Locale& locale,
    const TvDbId& id,
    SeasonOrder order,
    TheTvDbApi::ApiPage page,
    TheTvDbApi::ApiCallback callback)
{
    Q_UNUSED(order);
    sendGetRequest(locale, getEpisodesUrl(id, page), callback);
}

void TheTvDbApi::loadEpisode(const Locale& locale, const TvDbId& episodeId, ApiCallback callback)
{
    sendGetRequest(locale, getEpisodeUrl(episodeId), callback);
}

void TheTvDbApi::addHeadersToRequest(const Locale& locale, QNetworkRequest& request)
{
    request.setRawHeader("Accept-Language", locale.toString('-').toLocal8Bit());
    request.setRawHeader("Authorization", m_token.toBearer());
}

QUrl TheTvDbApi::makeFullUrl(const QString& suffix)
{
    return QUrl("https://api.thetvdb.com" + suffix);
}

QUrl TheTvDbApi::makeFullAssetUrl(const QString& suffix)
{
    return QUrl("https://www.thetvdb.com" + suffix);
}

QUrl TheTvDbApi::getShowSearchUrl(const QString& searchStr) const
{
    const QRegularExpression rx("^id(\\d+)$");
    QRegularExpressionMatch match = rx.match(searchStr);
    if (match.hasMatch()) {
        return TheTvDbApi::makeFullUrl("/series/" + match.captured(1));
    }

    QUrlQuery queries;
    queries.addQueryItem("name", searchStr);
    return TheTvDbApi::makeFullUrl("/search/series?" + queries.toString());
}

QUrl TheTvDbApi::getShowUrl(ApiShowDetails type, const TvDbId& id) const
{
    const QString typeStr = [type]() {
        switch (type) {
        case ApiShowDetails::ACTORS: return QStringLiteral("/actors");
        case ApiShowDetails::INFOS: return QString{};
        }
        qWarning() << "[TheTvDbApi] Unknown ApiShowDetails";
        return QString{};
    }();

    return TheTvDbApi::makeFullUrl(QStringLiteral("/series/%1%2").arg(id.toString(), typeStr));
}

QUrl TheTvDbApi::getImagesUrl(ShowScraperInfo type, const TvDbId& id) const
{
    const QString typeStr = [type]() {
        switch (type) {
        case ShowScraperInfo::Fanart: return QStringLiteral("fanart");
        case ShowScraperInfo::Poster: return QStringLiteral("poster");
        case ShowScraperInfo::SeasonPoster: return QStringLiteral("season");
        case ShowScraperInfo::SeasonBanner: return QStringLiteral("seasonwide");
        case ShowScraperInfo::Banner: return QStringLiteral("series");
        default: qWarning() << "[TheTvDbApi] Invalid image type"; return QStringLiteral("invalid");
        }
    }();

    return TheTvDbApi::makeFullUrl(QStringLiteral("/series/%1/images/query?keyType=%2").arg(id.toString(), typeStr));
}

QUrl TheTvDbApi::getEpisodesUrl(const TvDbId& showId, ApiPage page) const
{
    const auto path = QStringLiteral("/series/%1/episodes?page=%2").arg(showId.toString(), QString::number(page));
    return TheTvDbApi::makeFullUrl(path);
}

QUrl TheTvDbApi::getSeasonUrl(const TvDbId& showId, SeasonNumber season, SeasonOrder order) const
{
    const auto path = QStringLiteral("/series/%1/episodes/query?%2=%3")
                          .arg(showId.toString(), seasonOrderToUrlArg(order), season.toString());
    return TheTvDbApi::makeFullUrl(path);
}

QUrl TheTvDbApi::getEpisodeUrl(const TvDbId& episodeId) const
{
    return TheTvDbApi::makeFullUrl(QStringLiteral("/episodes/%1").arg(episodeId.toString()));
}

QString TheTvDbApi::seasonOrderToUrlArg(SeasonOrder order) const
{
    switch (order) {
    case SeasonOrder::Dvd: return "dvdSeason";
    case SeasonOrder::Aired: return "airedSeason";
    }
    qCritical() << "[TheTvDbApi] Unhandled SeasonOrder case!";
    return "airedSeason";
}


} // namespace scraper
} // namespace mediaelch
