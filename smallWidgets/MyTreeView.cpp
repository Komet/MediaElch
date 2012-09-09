#include "MyTreeView.h"

#include "Globals.h"

/**
 * @brief MyTreeView::MyTreeView
 * @param parent
 */
MyTreeView::MyTreeView(QWidget *parent) :
    QTreeView(parent)
{
}

/**
 * @brief Just skip drawing of the branches
 * @param painter
 * @param rect
 * @param index
 */
void MyTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(index);
}
