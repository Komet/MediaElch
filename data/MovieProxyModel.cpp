#include "MovieProxyModel.h"

#include <QDebug>
#include "globals/Globals.h"
#include "globals/Manager.h"

/**
 * @brief MovieProxyModel::MovieProxyModel
 * @param parent
 */
MovieProxyModel::MovieProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    m_sortBy{SortByNew}
{
    sort(0, Qt::AscendingOrder);
}

/**
 * @brief Checks if a row accepts the filter. Checks the first two "columns" of our model (Movie name and folder name)
 * @param sourceRow
 * @param sourceParent
 * @return Filter is accepted or not
 */
bool MovieProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    QList<Movie*> movies = Manager::instance()->movieModel()->movies();
    if (sourceRow < 0 || sourceRow >= movies.count())
        return true;

    Movie *movie = movies.at(sourceRow);
    foreach (Filter *filter, m_filters) {
        if (!filter->accepts(movie))
            return false;
    }

    return true;
}

/**
 * @brief Sort function for the movie model. Sorts movies by name and new files to top.
 * @param left
 * @param right
 * @return
 */
bool MovieProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    int cmp = QString::localeAwareCompare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString());

    if (m_sortBy == SortByAdded) {
        // Qt::UserRole+5
        return sourceModel()->data(left, Qt::UserRole+5).toDateTime() >= sourceModel()->data(right, Qt::UserRole+5).toDateTime();
    }

    if (m_sortBy == SortBySeen) {
        // Qt::UserRole+4
        if (sourceModel()->data(left, Qt::UserRole+4).toBool() && !sourceModel()->data(right, Qt::UserRole+4).toBool() )
            return false;
        if (!sourceModel()->data(left, Qt::UserRole+4).toBool() && sourceModel()->data(right, Qt::UserRole+4).toBool() )
            return true;
    }

    if (m_sortBy == SortByYear) {
        // Qt::UserRole+3
        if (sourceModel()->data(left, Qt::UserRole+3).toDate().year() != sourceModel()->data(right, Qt::UserRole+3).toDate().year())
            return sourceModel()->data(left, Qt::UserRole+3).toDate().year() >= sourceModel()->data(right, Qt::UserRole+3).toDate().year();
    }

    if (m_sortBy == SortByNew) {
        // Qt::UserRole+1
        if (sourceModel()->data(left, Qt::UserRole+1).toBool() && !sourceModel()->data(right, Qt::UserRole+1).toBool() )
            return false;
        if (!sourceModel()->data(left, Qt::UserRole+1).toBool() && sourceModel()->data(right, Qt::UserRole+1).toBool() )
            return true;
    }

    return (cmp < 0);
}

/**
 * @brief Sets active filters
 * @param filters
 * @param text
 */
void MovieProxyModel::setFilter(QList<Filter*> filters, QString text)
{
    m_filters = filters;
    m_filterText = text;
}

/**
 * @brief Sets sort by
 * @param sortBy Sort by
 */
void MovieProxyModel::setSortBy(SortBy sortBy)
{
    m_sortBy = sortBy;
    invalidate();
    sort(0, Qt::AscendingOrder);
}
