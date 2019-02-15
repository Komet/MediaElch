#include "globals/JsonRequest.h"

#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

#include "globals/NetworkReplyWatcher.h"

namespace MediaElch {

JsonPostRequest::JsonPostRequest(QUrl url, QJsonObject body)
{
    QNetworkAccessManager qnam;

    auto request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = qnam.post(request, QJsonDocument(body).toJson());
    NetworkReplyWatcher(nullptr, reply);

    QObject::connect(reply, &QNetworkReply::finished, [&]() {
        reply->deleteLater();
        QJsonDocument parsedJson;

        if (reply->error() == QNetworkReply::NoError) {
            QJsonParseError parseError;
            parsedJson = QJsonDocument::fromJson(reply->readAll(), &parseError);

            if (parseError.error != QJsonParseError::NoError) {
                qWarning() << "JSON error when parsing TheTvDb API key";
            }

        } else {
            qWarning() << "Network Error" << reply->errorString();
        }

        emit jsonParsed(parsedJson);
    });
}
} // namespace MediaElch
