#include "network/NetworkManager.h"

#include "network/NetworkReplyWatcher.h"

namespace mediaelch {
namespace network {

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
