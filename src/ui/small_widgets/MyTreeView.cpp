#include "MyTreeView.h"

#include "globals/Globals.h"

MyTreeView::MyTreeView(QWidget* parent) : QTreeView(parent)
{
}

/**
 * @brief Just skip drawing of the branches
 */
void MyTreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(index);
}
