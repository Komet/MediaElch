#pragma once

#include <QObject>
#include <QStatusBar>
#include <QSystemTrayIcon>

class Notificator : public QObject
{
    Q_OBJECT
public:
    explicit Notificator(QSystemTrayIcon* trayIcon = nullptr, QWidget* parent = nullptr);
    static Notificator* instance(QSystemTrayIcon* trayIcon = nullptr, QWidget* parent = nullptr);

    enum class Class
    {
        Information,
        Warning,
        Critical
    };

public slots:
    virtual void notify(Notificator::Class cls,
        const QString& title,
        const QString& text,
        const QIcon& icon = QIcon(),
        int timeout = 10000);

private:
    enum class Mode
    {
        None,
        QSystemTray,
        UserNotificationCenter
    };

    Mode m_mode;
    QWidget* m_parent;
    QSystemTrayIcon* m_trayIcon;
    void notifySystray(Class cls, const QString& title, const QString& text, const QIcon& icon, int timeout);
    void notifyMacUserNotificationCenter(Class cls, const QString& title, const QString& text, const QIcon& icon);
};
