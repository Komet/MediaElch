#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QTimer>
#include <QWidget>
#include "Message.h"

namespace Ui {
class MessageBox;
}

class MessageBox : public QWidget
{
    Q_OBJECT

public:
    explicit MessageBox(QWidget *parent = 0);
    ~MessageBox();
    static MessageBox *instance(QWidget *parent = 0);
    void reposition(QSize size);
    int showMessage(QString message, int timeout = 3000);
    void showProgressBar(QString message, int id);
    void hideProgressBar(int id);
    void progressBarProgress(int current, int max, int id);
private slots:
    void removeMessage(int id);
private:
    Ui::MessageBox *ui;
    QSize m_parentSize;
    int m_msgCounter;
    QList<Message*> m_messages;
    void adjustSize();
};

#endif // MESSAGEBOX_H
