#ifndef MACNOTIFICATIONHANDLER_H
#define MACNOTIFICATIONHANDLER_H

#include <QObject>

class MacNotificationHandler : public QObject
{
    Q_OBJECT

public:
    void showNotification(const QString &title, const QString &text);
    bool hasUserNotificationCenterSupport();
    static MacNotificationHandler *instance();
};

#endif // MACNOTIFICATIONHANDLER_H
