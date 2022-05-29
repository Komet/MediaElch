#include "ImdbApi.h"

#include "Version.h"
#include "globals/Meta.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"

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
    if (m_cache.hasValidElement(url, locale)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, this, [cb = std::move(callback), element = m_cache.getElement(url, locale)]() { //
            cb(element, {});
        });
        return;
    }

    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    addHeadersToRequest(locale, request);

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), locale, this]() {
        auto dls = makeDeleteLaterScope(reply);
        QString html;
        if (reply->error() == QNetworkReply::NoError) {
            html = QString::fromUtf8(reply->readAll());

            if (!html.isEmpty()) {
                m_cache.addElement(reply->url(), locale, html);
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
        case PageKind::PlotSummary: return "plotsummaryre";
        case PageKind::ReleaseInfo: return "releaseinfo";
        case PageKind::Keywords: return "keywords";
        }
        qCCritical(generic, "[ImdbApi] Unhandled page key!");
        return "";
    }();
    return makeFullUrl(QStringLiteral("/title/%1/%2").arg(id.toString(), pageStr));
}

QUrl ImdbApi::makeMovieSearchUrl(const QString& searchStr, bool includeAdult) const
{
    if (includeAdult) {
        QUrlQuery queries;
        queries.addQueryItem("adult", "include");
        queries.addQueryItem("title_type", "feature,documentary,tv_movie,short,video"); // Movie categories
        queries.addQueryItem("view", "simple");
        queries.addQueryItem("count", "100");
        queries.addQueryItem("title", searchStr);
        return makeFullUrl("/search/title/?" + queries.toString());
    }
    QUrlQuery queries;
    queries.addQueryItem("s", "tt");
    queries.addQueryItem("ttype", "ft"); // Movie category
    queries.addQueryItem("ref_", "fn_ft");
    queries.addQueryItem("q", searchStr);
    return makeFullUrl("/find?" + queries.toString());
}

QUrl ImdbApi::makeFullUrl(const QString& suffix)
{
    return QUrl("https://www.imdb.com" + suffix);
}

QUrl ImdbApi::makeFullAssetUrl(const QString& suffix)
{
    return QUrl("https://www.imdb.com" + suffix);
}

QUrl ImdbApi::makeShowSearchUrl(const QString& searchStr) const
{
    if (ImdbId::isValidFormat(searchStr)) {
        return makeFullUrl(QStringLiteral("/title/") + searchStr + '/');
    }

    QUrlQuery queries;
    queries.addQueryItem("s", "tt");
    queries.addQueryItem("ttype", "tv"); // TV category
    queries.addQueryItem("q", searchStr);
    return makeFullUrl("/find?" + queries.toString());
}

QUrl ImdbApi::makeSeasonUrl(const ImdbId& showId, SeasonNumber season) const
{
    QUrlQuery queries;
    queries.addQueryItem("season", season.toString());
    return makeFullUrl(QStringLiteral("/title/") + showId.toString() + //
                       QStringLiteral("/episodes?") + queries.toString());
}

QUrl ImdbApi::makeDefaultEpisodesUrl(const ImdbId& showId) const
{
    return makeFullUrl(QStringLiteral("/title/") + showId.toString() + QStringLiteral("/episodes"));
}


} // namespace scraper
} // namespace mediaelch
