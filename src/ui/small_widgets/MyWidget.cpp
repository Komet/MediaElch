#include "MyWidget.h"

MyWidget::MyWidget(QWidget* parent) : QWidget(parent)
{
}

void MyWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    emit resized();
}
