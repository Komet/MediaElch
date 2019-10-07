#pragma once

#include "globals/NetworkReplyWatcher.h"
#include "scrapers/tv_show/TheTvDb/Cache.h"

#include <QByteArray>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

#include <functional>

namespace thetvdb {

/**
 * @brief An ApiToken represents an API token from TheTvDb. These tokens
 * are JSON web tokens and are valid for about 24h
 * (according to https://api.thetvdb.com/swagger).
 */
class ApiToken
{
public:
    ApiToken() = default;
    explicit ApiToken(QString token) : m_token(std::move(token)) {}
    bool isValid() { return !m_token.isEmpty(); }
    QByteArray toBearer() { return "Bearer " + m_token.toLocal8Bit(); }

private:
    QString m_token;
};

/**
 * @brief Request to TheTvDb API. Requests a JSON web token (API token) the
 * first time the API is requested.
 */
class ApiRequest : public QObject
{
    Q_OBJECT

public:
    explicit ApiRequest(const QString& language) : m_language{language} {}

    static QUrl getFullUrl(const QString& suffix);
    static QUrl getFullAssetUrl(const QString& suffix);

    void sendGetRequest(const QUrl& url, std::function<void(QString)> callback);

private:
    void obtainJsonWebToken(std::function<void()> callback);
    void addHeadersToRequest(QNetworkRequest& request);

    const QString m_language;
    QNetworkAccessManager m_qnam;
};

} // namespace thetvdb
