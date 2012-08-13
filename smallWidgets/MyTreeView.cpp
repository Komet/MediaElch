#include "MyTreeView.h"

MyTreeView::MyTreeView(QWidget *parent) :
    QTreeView(parent)
{
}

void MyTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(index);
}
