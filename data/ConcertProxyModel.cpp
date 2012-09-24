#include "ConcertProxyModel.h"

#include <QDebug>
#include "globals/Globals.h"

/**
 * @brief ConcertProxyModel::ConcertProxyModel
 * @param parent
 */
ConcertProxyModel::ConcertProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

/**
 * @brief Checks if a row accepts the filter. Checks the first two "columns" of our model (Concert name and folder name)
 * @param sourceRow
 * @param sourceParent
 * @return Filter is accepted or not
 */
bool ConcertProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);

    return (sourceModel()->data(index0).toString().contains(filterRegExp()) ||
            sourceModel()->data(index1).toString().contains(filterRegExp()));
}

/**
 * @brief Sort function for the concert model. Sorts concerts by name and new files to top.
 * @param left
 * @param right
 * @return
 */
bool ConcertProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (sourceModel()->data(left, Qt::UserRole+1).toBool() && !sourceModel()->data(right, Qt::UserRole+1).toBool() )
        return true;
    if (!sourceModel()->data(left, Qt::UserRole+1).toBool() && sourceModel()->data(right, Qt::UserRole+1).toBool() )
        return false;
    int cmp = QString::compare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString());
    return !(cmp < 0);
}
