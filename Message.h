#ifndef MESSAGE_H
#define MESSAGE_H

#include <QTimer>
#include <QWidget>

namespace Ui {
class Message;
}

class Message : public QWidget
{
    Q_OBJECT

public:
    explicit Message(QWidget *parent = 0);
    ~Message();
    void setMessage(QString message, int timeout = 3000);
    void showProgressBar(bool show);
    void setProgress(int current, int max);
    void setId(int id);
    int id();

signals:
    void sigHideMessage(int);
private slots:
    void timeout();
private:
    Ui::Message *ui;
    int m_id;
    QTimer *m_timer;
};

#endif // MESSAGE_H
