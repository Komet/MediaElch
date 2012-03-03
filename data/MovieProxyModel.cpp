#include "MovieProxyModel.h"

#include <QDebug>

MovieProxyModel::MovieProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool MovieProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);

    return (sourceModel()->data(index0).toString().contains(filterRegExp()) ||
            sourceModel()->data(index1).toString().contains(filterRegExp()));
}

bool MovieProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (sourceModel()->data(left, Qt::UserRole+1).toBool() && !sourceModel()->data(right, Qt::UserRole+1).toBool() )
        return true;
    if (!sourceModel()->data(left, Qt::UserRole+1).toBool() && sourceModel()->data(right, Qt::UserRole+1).toBool() )
        return false;
    return !QString::compare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString());
}
