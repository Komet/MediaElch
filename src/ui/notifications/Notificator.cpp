#include "Notificator.h"

#include <QMessageBox>
#ifdef Q_OS_MAC
#    include "ui/notifications/MacNotificationHandler.h"
#    include <ApplicationServices/ApplicationServices.h>
#endif

Notificator::Notificator(QSystemTrayIcon* trayIcon, QWidget* parent) :
    QObject(parent), m_mode(Mode::None), m_parent(parent), m_trayIcon(trayIcon)
{
    if ((m_trayIcon != nullptr) && QSystemTrayIcon::supportsMessages()) {
        m_mode = Mode::QSystemTray;
    }
#ifdef Q_OS_MAC
    if (MacNotificationHandler::instance()->hasUserNotificationCenterSupport()) {
        m_mode = UserNotificationCenter;
    }
#endif
}

void Notificator::notifySystray(Class cls, const QString& title, const QString& text, const QIcon& icon, int timeout)
{
    Q_UNUSED(icon);
    QSystemTrayIcon::MessageIcon sicon = QSystemTrayIcon::NoIcon;
    switch (cls) {
    case Class::Information: sicon = QSystemTrayIcon::Information; break;
    case Class::Warning: sicon = QSystemTrayIcon::Warning; break;
    case Class::Critical: sicon = QSystemTrayIcon::Critical; break;
    }
    m_trayIcon->showMessage(title, text, sicon, timeout);
}

void Notificator::notifyMacUserNotificationCenter(Notificator::Class cls,
    const QString& title,
    const QString& text,
    const QIcon& icon)
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

void Notificator::notify(Class cls, const QString& title, const QString& text, const QIcon& icon, int timeout)
{
    switch (m_mode) {
    case Mode::QSystemTray: notifySystray(cls, title, text, icon, timeout); break;
#ifdef Q_OS_MAC
    case Mode::UserNotificationCenter: notifyMacUserNotificationCenter(cls, title, text, icon); break;
#endif
    default:
        if (cls == Class::Critical) {
            QMessageBox::critical(m_parent, title, text, QMessageBox::Ok, QMessageBox::Ok);
        }
        break;
    }
}

Notificator* Notificator::instance(QSystemTrayIcon* trayIcon, QWidget* parent)
{
    static Notificator* m_instance = nullptr;
    if (m_instance == nullptr) {
        m_instance = new Notificator(trayIcon, parent);
    }
    return m_instance;
}
