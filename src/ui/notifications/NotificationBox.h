#pragma once

#include "globals/Globals.h"
#include "ui/main/Message.h"

#include <QSize>
#include <QString>
#include <QTimer>
#include <QWidget>

#include <chrono>

namespace Ui {
class NotificationBox;
}

class NotificationBox : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationBox(QWidget* parent = nullptr);
    ~NotificationBox() override;
    static NotificationBox* instance(QWidget* parent = nullptr);
    void reposition(QSize size);

    int showMessage(QString message,
        NotificationType type = NotificationType::NotificationInfo,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{5000});

    int showSuccess(const QString& message, std::chrono::milliseconds timeout = std::chrono::milliseconds{5000})
    {
        return showMessage(message, NotificationType::NotificationSuccess, timeout);
    }

    int showError(const QString& message, std::chrono::milliseconds timeout = std::chrono::milliseconds{5000})
    {
        return showMessage(message, NotificationType::NotificationError, timeout);
    }

    int showWarning(const QString& message, std::chrono::milliseconds timeout = std::chrono::milliseconds{5000})
    {
        return showMessage(message, NotificationType::NotificationWarning, timeout);
    }

    int showInfo(const QString& message, std::chrono::milliseconds timeout = std::chrono::milliseconds{5000})
    {
        return showMessage(message, NotificationType::NotificationInfo, timeout);
    }

    void showProgressBar(QString message, int id, bool unique = false);
    int addProgressBar(QString message);
    void hideProgressBar(int id);
    void progressBarProgress(int current, int max, int id);
    int maxValue(int id);
    int value(int id);

public slots:
    virtual void removeMessage(int id);

private:
    void adjustSize();

private:
    Ui::NotificationBox* ui;
    QSize m_parentSize;
    int m_msgCounter = 0;
    QVector<Message*> m_messages;
};
