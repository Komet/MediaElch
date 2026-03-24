#pragma once

#include "data/ImdbId.h"
#include "data/Locale.h"
#include "network/NetworkManager.h"
#include "scrapers/ScraperError.h"
#include "utils/Meta.h"

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUrl>
#include <functional>

namespace mediaelch {
namespace scraper {

/// \brief API interface for IMDB using the GraphQL and Suggest APIs.
class ImdbApi : public QObject
{
    Q_OBJECT

public:
    explicit ImdbApi(QObject* parent = nullptr);
    ~ImdbApi() override = default;

    void initialize();
    ELCH_NODISCARD bool isInitialized() const;

public:
    using ApiCallback = std::function<void(QString, ScraperError)>;

    /// \brief Search using the IMDB Suggest API (JSON, no auth).
    void suggestSearch(const QString& query, ApiCallback callback);

    /// \brief Send a GraphQL query to graphql.imdb.com.
    void sendGraphQLRequest(const QString& query, const QJsonObject& variables, ApiCallback callback);

    /// \brief Load full title details via GraphQL.
    void loadTitleViaGraphQL(const ImdbId& id, ApiCallback callback);

    /// \brief Load all episodes for a title via GraphQL.
    void loadEpisodesViaGraphQL(const ImdbId& showId, int limit, ApiCallback callback);

signals:
    void initialized();

public:
    ELCH_NODISCARD static QUrl makeFullUrl(const QString& suffix);
    ELCH_NODISCARD static QUrl makeFullAssetUrl(const QString& suffix);

private:
    ELCH_NODISCARD static QUrl makeSuggestUrl(const QString& query);
    ELCH_NODISCARD static QUrl makeGraphQLUrl();

private:
    mediaelch::network::NetworkManager m_network;
};

} // namespace scraper
} // namespace mediaelch
