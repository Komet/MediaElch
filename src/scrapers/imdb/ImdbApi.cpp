#include "ImdbApi.h"

#include "Version.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "utils/Meta.h"

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
    QUrl url{makeShowSearchUrl(query)};
    sendGetRequest(locale, url, std::move(callback));
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
    QUrlQuery queries;
    queries.addQueryItem("title", searchStr);
    queries.addQueryItem("title_type", "tv_series,tv_miniseries");
    queries.addQueryItem("view", "simple");
    queries.addQueryItem("count", "100");
    return makeFullUrl("/search/title/?" + queries.toString());
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
