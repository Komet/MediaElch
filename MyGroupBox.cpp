#include "MyGroupBox.h"

MyGroupBox::MyGroupBox(QWidget *parent) :
    QGroupBox(parent)
{
}

void MyGroupBox::resizeEvent(QResizeEvent *event)
{
    emit resized(event->size());
    QGroupBox::resizeEvent(event);
}
