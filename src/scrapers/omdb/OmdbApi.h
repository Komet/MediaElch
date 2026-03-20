#pragma once

#include "data/ImdbId.h"
#include "network/NetworkManager.h"
#include "scrapers/ScraperError.h"
#include "utils/Meta.h"

#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <QUrl>
#include <functional>

namespace mediaelch {
namespace scraper {

/// \brief API interface for the Open Movie Database (OMDb).
/// \details OMDb provides movie/TV metadata via a simple REST API.
///          Requires a personal API key (free: 1000 requests/day).
///          Returns JSON. English only.
class OmdbApi : public QObject
{
    Q_OBJECT

public:
    explicit OmdbApi(QObject* parent = nullptr);
    ~OmdbApi() override = default;

    void initialize();
    ELCH_NODISCARD bool isInitialized() const;

    void setApiKey(const QString& apiKey);

public:
    using ApiCallback = std::function<void(QJsonDocument, ScraperError)>;

    void sendGetRequest(const QUrl& url, ApiCallback callback);

    void searchForMovie(const QString& query, int year, int page, ApiCallback callback);
    void searchForShow(const QString& query, int year, int page, ApiCallback callback);

    void loadMovie(const ImdbId& imdbId, ApiCallback callback);
    void loadShow(const ImdbId& imdbId, ApiCallback callback);
    void loadSeason(const ImdbId& showId, int season, ApiCallback callback);
    void loadEpisode(const ImdbId& episodeId, ApiCallback callback);

private:
    QUrl makeApiUrl(const QUrlQuery& query) const;

    mediaelch::network::NetworkManager m_network;
    QString m_apiKey;
};

} // namespace scraper
} // namespace mediaelch
