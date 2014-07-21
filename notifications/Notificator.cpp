#include "Notificator.h"

#include <QMessageBox>
#ifdef Q_OS_MAC
    #include <ApplicationServices/ApplicationServices.h>
    #include "notifications/MacNotificationHandler.h"
#endif

Notificator::Notificator(QSystemTrayIcon *trayIcon, QWidget *parent):
    QObject(parent),
    m_mode(None),
    m_parent(parent),
    m_trayIcon(trayIcon)
{
    if (m_trayIcon && m_trayIcon->supportsMessages())
        m_mode = QSystemTray;
#ifdef Q_OS_MAC
    if (MacNotificationHandler::instance()->hasUserNotificationCenterSupport())
        m_mode = UserNotificationCenter;
#endif
}

void Notificator::notifySystray(Class cls, const QString &title, const QString &text, const QIcon &icon, int timeout)
{
    Q_UNUSED(icon);
    QSystemTrayIcon::MessageIcon sicon = QSystemTrayIcon::NoIcon;
    switch(cls) {
    case Information:
        sicon = QSystemTrayIcon::Information;
        break;
    case Warning:
        sicon = QSystemTrayIcon::Warning;
        break;
    case Critical:
        sicon = QSystemTrayIcon::Critical;
        break;
    }
    m_trayIcon->showMessage(title, text, sicon, timeout);
}

void Notificator::notifyMacUserNotificationCenter(Class cls, const QString &title, const QString &text, const QIcon &icon)
{
    Q_UNUSED(cls);
    Q_UNUSED(icon);
#ifdef Q_OS_MAC
    MacNotificationHandler::instance()->showNotification(title, text);
#else
    Q_UNUSED(title);
    Q_UNUSED(text);
#endif
}

void Notificator::notify(Class cls, const QString &title, const QString &text, const QIcon &icon, int timeout)
{
    switch(m_mode) {
    case QSystemTray:
        notifySystray(cls, title, text, icon, timeout);
        break;
    #ifdef Q_OS_MAC
    case UserNotificationCenter:
        notifyMacUserNotificationCenter(cls, title, text, icon);
        break;
    #endif
    default:
        if (cls == Critical)
            QMessageBox::critical(m_parent, title, text, QMessageBox::Ok, QMessageBox::Ok);
        break;
    }
}

Notificator *Notificator::instance(QSystemTrayIcon *trayIcon, QWidget *parent)
{
    static Notificator *m_instance = 0;
    if (!m_instance)
        m_instance = new Notificator(trayIcon, parent);
    return m_instance;
}
