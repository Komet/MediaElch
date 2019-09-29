#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>

namespace MediaElch {

/**
 * @brief Send a json post request.
 * @example
 *   auto* req = new JsonPostReqiest(url, body);
 *   connext(req, &JsonPostRequest::sigResponse, this, [&]() {
 *     req->deleteLater();
 *   });
 */
class JsonPostRequest : public QObject
{
    Q_OBJECT

public:
    explicit JsonPostRequest(QUrl url, QJsonObject body);

signals:
    void sigResponse(QJsonDocument& document);

private:
    QNetworkAccessManager m_qnam;
};

} // namespace MediaElch
