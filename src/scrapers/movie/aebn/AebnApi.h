#pragma once

#include "network/NetworkManager.h"
#include "network/WebsiteCache.h"
#include "scrapers/ScraperError.h"

#include <QObject>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <functional>

namespace mediaelch {
namespace scraper {

/// \brief API interface for TheTvDb
class AebnApi : public QObject
{
    Q_OBJECT

public:
    explicit AebnApi(QObject* parent = nullptr);
    ~AebnApi() override = default;

public:
    using ApiCallback = std::function<void(QString, ScraperError)>;

    void sendGetRequest(const QUrl& url, const Locale& locale, ApiCallback callback);

    void searchForMovie(const QString& query, const Locale& locale, const QString& genre, ApiCallback callback);
    void loadMovie(const QString& id, const Locale& locale, const QString& genre, ApiCallback callback);
    void loadActor(const QString& id, const Locale& locale, const QString& genre, ApiCallback callback);

public:
    static QUrl makeFullUrl(const QString& suffix);

private:
    QUrl makeApiUrl(const QString& suffix, const Locale& locale, QUrlQuery query) const;
    QUrl makeMovieSearchUrl(const QString& searchStr, const QString& genre, const Locale& locale) const;
    QUrl makeMovieUrl(const QString& id, const QString& genre, const Locale& locale) const;
    QUrl makeActorUrl(const QString& id, const QString& genre, const Locale& locale) const;

private:
    mediaelch::network::NetworkManager m_network;
    WebsiteCache m_cache;
};

} // namespace scraper
} // namespace mediaelch
