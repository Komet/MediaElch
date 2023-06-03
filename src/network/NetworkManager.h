#pragma once

#include "network/WebsiteCache.h"

#include <QAuthenticator>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>

namespace mediaelch {
namespace network {

/// \brief Wrapper around QNetworkAccessManager that adds timeout mechanisms and logging.
class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager() override = default;

public:
    /// \brief   Disables the network proxy when syncing with Kodi.
    /// \details Useful if Kodi is in the local network but a proxy should be
    ///          used for downloading artwork.
    void disableProxy();
    /// \brief Enables the default proxy used by MediaElch, i.e. the proxy set
    ///        in MediaElch's network settings.
    void enableDefaultProxy();

    QNetworkReply* get(const QNetworkRequest& request);
    QNetworkReply* getWithWatcher(const QNetworkRequest& request);

    QNetworkReply* post(const QNetworkRequest& request, const QByteArray& data);
    QNetworkReply* postWithWatcher(const QNetworkRequest& request, const QByteArray& data);

    // TODO: If possible, integrate cache with functions above.  Needs refactoring, because
    //       we can't simply return a QNetworkReply on our own.

    WebsiteCache& cache();

signals:
    void authenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);
    void finished(QNetworkReply* reply);

private:
    QNetworkAccessManager m_qnam;
    WebsiteCache m_cache;
};

} // namespace network
} // namespace mediaelch
