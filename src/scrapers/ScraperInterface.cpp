#include "scrapers/ScraperInterface.h"

#include "ui/notifications/NotificationBox.h"

#include <QCoreApplication>

class NetworkErrorNotification
{
    Q_DECLARE_TR_FUNCTIONS(NetworkErrorNotification)

public:
    NetworkErrorNotification() = default;

    static QString messageForNetworkError(QNetworkReply::NetworkError error, const QUrl& url)
    {
        const QString networkError = QStringLiteral("<b>") + tr("Network Error") + "</b><br/>";
        QString msg;
        switch (error) {
        case QNetworkReply::OperationCanceledError: msg = tr("Timeout by MediaElch"); break;
        case QNetworkReply::TimeoutError: msg = tr("Remote connection timed out"); break;
        case QNetworkReply::SslHandshakeFailedError: msg = tr("SSL handshake failed"); break;
        case QNetworkReply::TooManyRedirectsError: msg = tr("Too many redirects"); break;
        case QNetworkReply::ConnectionRefusedError: msg = tr("Connection refused by host"); break;
        case QNetworkReply::HostNotFoundError: msg = tr("Host not found"); break;
        case QNetworkReply::UnknownNetworkError: msg = tr("Unknown network error"); break;
        default: msg = tr("Could not load the requested resource"); break;
        }
        QString urlStr = url.toString(QUrl::FullyDecoded | QUrl::RemoveUserInfo | QUrl::RemoveQuery);
        return networkError + msg + "<br/><i>" + urlStr + "</i>";
    }
};

void ScraperInterface::showNetworkError(const QNetworkReply& reply)
{
    using namespace std::chrono_literals;

    QString message(NetworkErrorNotification::messageForNetworkError(reply.error(), reply.request().url()));
    NotificationBox::instance()->showError(message, 6s);
}
