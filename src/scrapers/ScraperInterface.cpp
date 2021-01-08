#include "scrapers/ScraperInterface.h"

#include "network/HttpStatusCodes.h"
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
        QString msg = mediaelch::translateNetworkError(error);
        QString urlStr = url.toString(QUrl::RemoveUserInfo | QUrl::RemoveQuery);
        return networkError + msg + "<br/><i>" + urlStr + "</i>";
    }
};

void ScraperInterface::showNetworkError(const QNetworkReply& reply)
{
    using namespace std::chrono_literals;

    QString message(NetworkErrorNotification::messageForNetworkError(reply.error(), reply.request().url()));
    NotificationBox::instance()->showError(message, 6s);
}

void ScraperInterface::showNetworkError(const mediaelch::ScraperError& error)
{
    using namespace std::chrono_literals;
    NotificationBox::instance()->showError(error.message, 6s);
}
