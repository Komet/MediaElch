#pragma once

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
    explicit NetworkManager(QObject* parent = nullptr) : QObject(parent) {}
    ~NetworkManager() override = default;

public:
    QNetworkReply* get(const QNetworkRequest& request);
    QNetworkReply* getWithWatcher(const QNetworkRequest& request);

    QNetworkReply* post(const QNetworkRequest& request, const QByteArray& data);
    QNetworkReply* postWithWatcher(const QNetworkRequest& request, const QByteArray& data);

signals:
    void authenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);

private:
    QNetworkAccessManager m_qnam;
};

} // namespace network
} // namespace mediaelch
