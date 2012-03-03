#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>
#include <QMouseEvent>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent = 0);
protected:
    void mousePressEvent(QMouseEvent *ev);
signals:
    void clicked();
};

#endif // MYLABEL_H
