#pragma once

#include <QTimer>
#include <QWidget>

namespace Ui {
class Message;
}

enum class NotificationType
{
    NotificationInfo,
    NotificationWarning,
    NotificationSuccess,
    NotificationError
};

/**
 * \brief The Message class
 * Instances of this widget are displayed in the MessageBox
 */
class Message : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int id READ id WRITE setId)

public:
    explicit Message(QWidget* parent = nullptr);
    ~Message() override;
    void setType(NotificationType type);
    void setMessage(QString message, int timeout = 3000);
    void showProgressBar(bool show);
    void setProgress(int current, int max);
    void setId(int id);
    int id() const;
    int maxValue() const;
    int value() const;

signals:
    void sigHideMessage(int);

private slots:
    void timeout();

private:
    Ui::Message* ui = nullptr;
    int m_id = 0;
    QTimer* m_timer = nullptr;
};
