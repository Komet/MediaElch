#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QTimer>
#include <QWidget>

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
    void showMessage(QString message);
    void showProgressBar(QString message);
    void hideProgressBar();
    void progressBarProgress(int current, int max);

private:
    Ui::MessageBox *ui;
    QTimer *m_timer;
    QSize m_parentSize;
};

#endif // MESSAGEBOX_H
