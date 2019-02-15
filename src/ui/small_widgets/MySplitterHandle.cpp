#include "MySplitterHandle.h"

#include <QPainter>

MySplitterHandle::MySplitterHandle(Qt::Orientation orientation, QSplitter* parent) :
    QSplitterHandle(orientation, parent)
{
}

void MySplitterHandle::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    const QColor lightGrey(235, 235, 235);
    painter.fillRect(event->rect(), QBrush(lightGrey));
}

QSize MySplitterHandle::sizeHint() const
{
    return (orientation() == Qt::Horizontal) ? QSize(1, height()) : QSize(width(), 1);
}
