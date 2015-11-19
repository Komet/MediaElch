#ifndef NOTIFICATIONBOX_H
#define NOTIFICATIONBOX_H

#include <QTimer>
#include <QWidget>
#include "globals/Globals.h"
#include "main/Message.h"

namespace Ui {
class NotificationBox;
}

class NotificationBox : public QWidget
{
    Q_OBJECT

public:
    enum NotificationType {
        NotificationInfo, NotificationWarning, NotificationSuccess, NotificationError
    };

    explicit NotificationBox(QWidget *parent = 0);
    ~NotificationBox();
    static NotificationBox *instance(QWidget *parent = 0);
    void reposition(QSize size);
    virtual int showMessage(QString message, NotificationBox::NotificationType type = NotificationInfo, int timeout = 5000);
    void showProgressBar(QString message, int id, bool unique = false);
    int addProgressBar(QString message);
    void hideProgressBar(int id);
    void progressBarProgress(int current, int max, int id);
    int maxValue(int id);
    int value(int id);

public slots:
    virtual void removeMessage(int id);

private:
    Ui::NotificationBox *ui;
    QSize m_parentSize;
    int m_msgCounter;
    QList<Message*> m_messages;
    void adjustSize();
};

#endif // NOTIFICATIONBOX_H
