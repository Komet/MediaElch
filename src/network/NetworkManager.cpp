#include "network/NetworkManager.h"

#include "network/NetworkReplyWatcher.h"

namespace mediaelch {
namespace network {

NetworkManager::NetworkManager(QObject* parent) : QObject(parent)
{
    // Mapping of important signals
    // clang-format off
    connect(&m_qnam, &QNetworkAccessManager::authenticationRequired, this, &NetworkManager::authenticationRequired, Qt::UniqueConnection);
    connect(&m_qnam, &QNetworkAccessManager::finished,               this, &NetworkManager::finished,               Qt::UniqueConnection);
    // clang-format on
}

QNetworkReply* NetworkManager::get(const QNetworkRequest& request)
{
    return m_qnam.get(request);
}

QNetworkReply* NetworkManager::getWithWatcher(const QNetworkRequest& request)
{
    QNetworkReply* reply = m_qnam.get(request);
    new NetworkReplyWatcher(this, reply);
    return reply;
}

QNetworkReply* NetworkManager::post(const QNetworkRequest& request, const QByteArray& data)
{
    return m_qnam.post(request, data);
}

QNetworkReply* NetworkManager::postWithWatcher(const QNetworkRequest& request, const QByteArray& data)
{
    QNetworkReply* reply = m_qnam.post(request, data);
    new NetworkReplyWatcher(this, reply);
    return reply;
}

} // namespace network
} // namespace mediaelch
