#include "scrapers/tv_show/tvmaze/TvMazeApi.h"

#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "utils/Meta.h"

#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace mediaelch {
namespace scraper {

TvMazeApi::TvMazeApi(QObject* parent) : QObject(parent)
{
}

QString TvMazeApi::removeBasicHtmlElements(QString str)
{
    str.replace("<p>", "");
    str.replace("</p>", "");
    str.replace("<b>", "");
    str.replace("</b>", "");
    str.replace("<i>", "");
    str.replace("</i>", "");
    return str;
}

void TvMazeApi::sendGetRequest(const QUrl& url, TvMazeApi::ApiCallback callback)
{
    QNetworkRequest request = mediaelch::network::jsonRequestWithDefaults(url);
    if (m_network.cache().hasValidElement(request)) {
        // Do not immediately run the callback because classes higher up may
        // set up a Qt connection while the network request is running.
        QTimer::singleShot(0, this, [cb = std::move(callback), element = m_network.cache().getElement(request)]() {
            // should not result in a parse error because the cache element is
            // only stored if no error occurred at all.
            cb(QJsonDocument::fromJson(element.toUtf8()), {});
        });
        return;
    }

    QNetworkReply* reply = m_network.getWithWatcher(request);

    connect(reply, &QNetworkReply::finished, this, [reply, cb = std::move(callback), request, this]() {
        auto dls = makeDeleteLaterScope(reply);
        QString data;

        if (reply->error() == QNetworkReply::NoError) {
            data = QString::fromUtf8(reply->readAll());

        } else {
            qCWarning(generic) << "[TvMazeApi] Network Error:" << reply->errorString() << "for URL" << reply->url();
        }

        QJsonParseError parseError{};
        QJsonDocument json;
        if (!data.isEmpty()) {
            json = QJsonDocument::fromJson(data.toUtf8(), &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                m_network.cache().addElement(request, data);
            }
        }

        ScraperError error = makeScraperError(data, *reply, parseError);
        cb(json, error);
    });
}

void TvMazeApi::searchForShow(const QString& query, TvMazeApi::ApiCallback callback)
{
    sendGetRequest(makeShowSearchUrl(query), std::move(callback));
}

void TvMazeApi::loadShowInfos(const TvMazeId& id, TvMazeApi::ApiCallback callback)
{
    sendGetRequest(makeShowUrl(id), std::move(callback));
}

void TvMazeApi::loadEpisode(const TvMazeId& episodeId, TvMazeApi::ApiCallback callback)
{
    sendGetRequest(makeEpisodeUrl(episodeId), std::move(callback));
}

void TvMazeApi::loadAllEpisodes(const TvMazeId& showId, TvMazeApi::ApiCallback callback)
{
    sendGetRequest(makeAllEpisodesUrl(showId), std::move(callback));
}

QUrl TvMazeApi::makeShowSearchUrl(const QString& searchStr) const
{
    QUrlQuery queries;
    queries.addQueryItem("q", searchStr);
    return makeApiUrl("/search/shows", queries);
}

QUrl TvMazeApi::makeShowUrl(const TvMazeId& showId) const
{
    QUrlQuery queries;
    queries.addQueryItem("embed[]", "cast");
    queries.addQueryItem("embed[]", "crew");
    queries.addQueryItem("embed[]", "images");
    queries.addQueryItem("embed[]", "seasons");
    return makeApiUrl(QStringLiteral("/shows/%1").arg(showId.toString()), queries);
}

QUrl TvMazeApi::makeEpisodeUrl(const TvMazeId& episodeId) const
{
    QUrlQuery queries;
    //    queries.addQueryItem("embed[]", "cast");
    //    queries.addQueryItem("embed[]", "crew");
    //    queries.addQueryItem("embed[]", "images");
    queries.addQueryItem("embed[]", "guestcast");
    queries.addQueryItem("embed[]", "guestcrew");
    return makeApiUrl(QStringLiteral("/episodes/%1").arg(episodeId.toString()), queries);
}

QUrl TvMazeApi::makeAllEpisodesUrl(const TvMazeId& showId) const
{
    return makeApiUrl(QStringLiteral("/shows/%1/episodes").arg(showId.toString()), {});
}

QUrl TvMazeApi::makeApiUrl(const QString& suffix, const QUrlQuery& query) const
{
    return QStringLiteral("https://api.tvmaze.com%1?%2").arg(suffix, query.toString());
}

} // namespace scraper
} // namespace mediaelch
