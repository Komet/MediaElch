#include "MacNotificationHandler.h"

#undef slots
#include <Cocoa/Cocoa.h>
#include <QDebug>

void MacNotificationHandler::showNotification(const QString &title, const QString &text)
{
    if (hasUserNotificationCenterSupport()) {
        QByteArray utf8 = title.toUtf8();
        char* cString = (char *)utf8.constData();
        NSString *titleMac = [[NSString alloc] initWithUTF8String:cString];

        utf8 = text.toUtf8();
        cString = (char *)utf8.constData();
        NSString *textMac = [[NSString alloc] initWithUTF8String:cString];

        id userNotification = [[NSClassFromString(@"NSUserNotification") alloc] init];
        [userNotification performSelector:@selector(setTitle:) withObject:titleMac];
        [userNotification performSelector:@selector(setInformativeText:) withObject:textMac];

        id notificationCenterInstance = [NSClassFromString(@"NSUserNotificationCenter") performSelector:@selector(defaultUserNotificationCenter)];
        [notificationCenterInstance performSelector:@selector(deliverNotification:) withObject:userNotification];

        [titleMac release];
        [textMac release];
        [userNotification release];
    }
}

bool MacNotificationHandler::hasUserNotificationCenterSupport()
{
    Class possibleClass = NSClassFromString(@"NSUserNotificationCenter");
    return possibleClass!=nil;
}

MacNotificationHandler *MacNotificationHandler::instance()
{
    static MacNotificationHandler *m_instance = 0;
    if (!m_instance) {
        m_instance = new MacNotificationHandler();
    }
    return m_instance;
}
