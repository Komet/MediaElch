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
