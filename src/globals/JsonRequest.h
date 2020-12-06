#pragma once

#include "network/NetworkManager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QUrl>

namespace mediaelch {

/// \brief Send a json post request.
/// \example
///   auto* req = new JsonPostReqiest(url, body);
///   connext(req, &JsonPostRequest::sigResponse, this, [&]() {
///     req->deleteLater();
///   });
class JsonPostRequest : public QObject
{
    Q_OBJECT

public:
    explicit JsonPostRequest(QUrl url, QJsonObject body, QObject* parent = nullptr);

signals:
    void sigResponse(QJsonDocument& document);

private:
    network::NetworkManager m_network;
};

} // namespace mediaelch
