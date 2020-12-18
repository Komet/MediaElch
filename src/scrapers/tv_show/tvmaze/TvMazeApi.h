#pragma once

#include "network/HttpStatusCodes.h"
#include "network/NetworkManager.h"
#include "network/WebsiteCache.h"
#include "scrapers/ScraperError.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/TvMazeId.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>

#include <functional>

namespace mediaelch {
namespace scraper {

/// \brief API interface for TVMaze
class TvMazeApi : public QObject
{
    Q_OBJECT

public:
    explicit TvMazeApi(QObject* parent = nullptr);
    ~TvMazeApi() override = default;

public:
    /// \brief Removes basic HTML elements that are always part of certain
    ///        elements like "summary".  This is _no_ proper HTML escaping.
    static QString removeBasicHtmlElements(QString str);

public:
    using ApiCallback = std::function<void(QJsonDocument, ScraperError)>;

    void sendGetRequest(const QUrl& url, ApiCallback callback);

    void searchForShow(const QString& query, ApiCallback callback);

    void loadShowInfos(const TvMazeId& id, ApiCallback callback);
    void loadEpisode(const TvMazeId& episodeId, ApiCallback callback);
    void loadAllEpisodes(const TvMazeId& showId, ApiCallback callback);

private:
    QUrl makeApiUrl(const QString& suffix, const QUrlQuery& query) const;
    QUrl makeShowSearchUrl(const QString& searchStr) const;
    QUrl makeShowUrl(const TvMazeId& showId) const;
    QUrl makeEpisodeUrl(const TvMazeId& episodeId) const;
    QUrl makeAllEpisodesUrl(const TvMazeId& showId) const;

private:
    mediaelch::network::NetworkManager m_network;
    WebsiteCache m_cache;
};

} // namespace scraper
} // namespace mediaelch
