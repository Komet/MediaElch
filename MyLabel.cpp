#include "MyLabel.h"

MyLabel::MyLabel(QWidget *parent) :
    QLabel(parent)
{
}

void MyLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        emit clicked();
    QLabel::mousePressEvent(ev);
}
