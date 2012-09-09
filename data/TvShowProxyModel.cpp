#include "TvShowProxyModel.h"

#include "globals/Globals.h"
#include "globals/Manager.h"

/**
 * @brief TvShowProxyModel::TvShowProxyModel
 * @param parent
 */
TvShowProxyModel::TvShowProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

/**
 * @brief Checks if a row accepts the filter. Checks the first column of our model (TV Show name)
 * @param sourceRow
 * @param sourceParent
 * @return
 */
bool TvShowProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return (sourceModel()->data(index).toString().contains(filterRegExp()));
}

/**
 * @brief Sort function for the tv show model. Sorts tv shows by name.
 * @param left
 * @param right
 * @return
 */
bool TvShowProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    int cmp = QString::compare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString());
    return !(cmp < 0);
}
