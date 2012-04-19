#include "TvShowProxyModel.h"

#include "Manager.h"

TvShowProxyModel::TvShowProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool TvShowProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return (sourceModel()->data(index).toString().contains(filterRegExp()));
}

bool TvShowProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    int cmp = QString::compare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString());
    return !(cmp < 0);
}
