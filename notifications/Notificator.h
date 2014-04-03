#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H

#include <QObject>
#include <QStatusBar>
#include <QSystemTrayIcon>

class Notificator : public QObject
{
    Q_OBJECT
public:
    explicit Notificator(QSystemTrayIcon *trayIcon = 0, QWidget *parent = 0);
    static Notificator *instance(QSystemTrayIcon *trayIcon = 0, QWidget *parent = 0);

    enum Class {
        Information,
        Warning,
        Critical
    };

public slots:
    virtual void notify(Class cls, const QString &title, const QString &text, const QIcon &icon = QIcon(), int timeout = 10000);

private:
    enum Mode {
        None,
        QSystemTray,
        UserNotificationCenter
    };

    Mode m_mode;
    QWidget *m_parent;
    QSystemTrayIcon *m_trayIcon;
    void notifySystray(Class cls, const QString &title, const QString &text, const QIcon &icon, int timeout);
    void notifyMacUserNotificationCenter(Class cls, const QString &title, const QString &text, const QIcon &icon);
};

#endif // NOTIFICATOR_H
