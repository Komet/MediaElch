#include "MyTableView.h"
#include <QDebug>

MyTableView::MyTableView(QWidget *parent) :
    QTableView(parent)
{
}

void MyTableView::setLastColumnWidth(int &width)
{
    setColumnWidth(model()->columnCount()-1, width);
}

int MyTableView::lastColumnWidth() const
{
    return columnWidth(model()->columnCount()-1);
}

void MyTableView::setFirstColumnWidth(int &width)
{
    setColumnWidth(0, width);
}

int MyTableView::firstColumnWidth() const
{
    return columnWidth(0);
}

void MyTableView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->pos().x() > 0 && event->pos().x() < 30) {
        if (!m_mouseInLeftEdge) {
            m_mouseInLeftEdge = true;
            emit sigLeftEdge(true);
        }
    } else {
        if (m_mouseInLeftEdge) {
            m_mouseInLeftEdge = false;
            emit sigLeftEdge(false);
        }
    }
}
