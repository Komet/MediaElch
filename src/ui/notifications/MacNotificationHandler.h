#pragma once

#include <QObject>

class MacNotificationHandler : public QObject
{
    Q_OBJECT

public:
    void showNotification(const QString& title, const QString& text);
    bool hasUserNotificationCenterSupport();
    static MacNotificationHandler* instance();
};
