#include "ApiRequest.h"

#include "globals/JsonRequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

namespace thetvdb {

static ApiToken s_token;

QUrl ApiRequest::getFullUrl(const QString& suffix)
{
    return QUrl("https://api.thetvdb.com" + suffix);
}

QUrl ApiRequest::getFullAssetUrl(const QString& suffix)
{
    return QUrl("https://www.thetvdb.com" + suffix);
}

void ApiRequest::sendGetRequest(const QUrl& url, std::function<void(QString)> callback)
{
    if (thetvdb::hasValidCacheElement(url)) {
        callback(thetvdb::getCacheElement(url));
        return;
    }

    obtainJsonWebToken([=]() {
        QNetworkRequest request(url);
        addHeadersToRequest(request);

        QNetworkReply* reply(m_qnam.get(request));
        new NetworkReplyWatcher(this, reply);

        connect(reply, &QNetworkReply::finished, [reply, callback]() {
            QString data{"{}"};
            if (reply->error() == QNetworkReply::NoError) {
                data = QString::fromUtf8(reply->readAll());
                thetvdb::addCacheElement(reply->url(), data);

            } else {
                qWarning() << "[TheTvDb][ApiRequest] Network Error:" << reply->errorString() << "for URL"
                           << reply->url();
            }
            callback(data);
            reply->deleteLater();
        });
    });
}

void ApiRequest::obtainJsonWebToken(std::function<void()> callback)
{
    if (s_token.isValid()) {
        callback();
        return;
    }
    const QJsonObject body{{"apikey", "A0BB9A0F6762942B"}};
    auto* request = new MediaElch::JsonPostRequest(getFullUrl("/login"), body);
    connect(request, &MediaElch::JsonPostRequest::sigResponse, this, [request, callback](QJsonDocument& parsedJson) {
        request->deleteLater();
        qDebug() << "[TheTvDb] Received JSON web token";
        s_token = ApiToken(parsedJson.object().value("token").toString());
        callback();
    });
}

/**
 * @brief Add neccassaray headers for TheTvDb to the request object.
 * Token must exist.
 * @see ApiRequest::obtainJsonWebToken
 */
void ApiRequest::addHeadersToRequest(QNetworkRequest& request)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept-Language", m_language.toLocal8Bit());
    request.setRawHeader("Authorization", s_token.toBearer());
}

} // namespace thetvdb
