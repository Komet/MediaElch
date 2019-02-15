#pragma once

#include <QTreeView>

#include "globals/Globals.h"

/**
 * @brief The MyTreeView class
 * This is TreeView without branches
 */
class MyTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit MyTreeView(QWidget* parent = nullptr);

protected:
    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
};
