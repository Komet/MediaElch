#include "MyTableView.h"

MyTableView::MyTableView(QWidget *parent) :
    QTableView(parent)
{
}

void MyTableView::resizeEvent(QResizeEvent *event)
{
    emit resized(event->size());
    QTableView::resizeEvent(event);
}
