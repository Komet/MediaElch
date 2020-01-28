#include "MovieProxyModel.h"

#include <QDebug>

#include "globals/Filter.h"
#include "globals/Globals.h"
#include "globals/Manager.h"

MovieProxyModel::MovieProxyModel(QObject* parent) :
    QSortFilterProxyModel(parent), m_sortBy{SortBy::New}, m_filterDuplicates{false}
{
    sort(0, Qt::AscendingOrder);
}

/**
 * @brief Checks if a row accepts the filter. Checks the first two "columns" of our model (Movie name and folder name)
 * @return Filter is accepted or not
 */
bool MovieProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    Q_UNUSED(sourceParent);
    QVector<Movie*> movies = Manager::instance()->movieModel()->movies();
    if (sourceRow < 0 || sourceRow >= movies.count()) {
        return true;
    }

    Movie* movie = movies.at(sourceRow);
    for (Filter* filter : m_filters) {
        if (!filter->accepts(movie)) {
            return false;
        }
    }

    return !(m_filterDuplicates && !movies.at(sourceRow)->hasDuplicates());
}

/**
 * @brief Sort function for the movie model. Sorts movies by name and new files to top.
 */
bool MovieProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    int cmp = QString::localeAwareCompare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString());

    if (m_sortBy == SortBy::Added) {
        // Qt::UserRole+5
        return sourceModel()->data(left, Qt::UserRole + 5).toDateTime()
               >= sourceModel()->data(right, Qt::UserRole + 5).toDateTime();
    }

    if (m_sortBy == SortBy::Seen) {
        // Qt::UserRole+4
        if (sourceModel()->data(left, Qt::UserRole + 4).toBool()
            && !sourceModel()->data(right, Qt::UserRole + 4).toBool()) {
            return false;
        }
        if (!sourceModel()->data(left, Qt::UserRole + 4).toBool()
            && sourceModel()->data(right, Qt::UserRole + 4).toBool()) {
            return true;
        }
    }

    if (m_sortBy == SortBy::Year) {
        // Qt::UserRole+3
        if (sourceModel()->data(left, Qt::UserRole + 3).toDate().year()
            != sourceModel()->data(right, Qt::UserRole + 3).toDate().year()) {
            return sourceModel()->data(left, Qt::UserRole + 3).toDate().year()
                   >= sourceModel()->data(right, Qt::UserRole + 3).toDate().year();
        }
    }

    if (m_sortBy == SortBy::New) {
        // Qt::UserRole+1
        if (sourceModel()->data(left, Qt::UserRole + 1).toBool()
            && !sourceModel()->data(right, Qt::UserRole + 1).toBool()) {
            return false;
        }
        if (!sourceModel()->data(left, Qt::UserRole + 1).toBool()
            && sourceModel()->data(right, Qt::UserRole + 1).toBool()) {
            return true;
        }
    }

    return (cmp < 0);
}

bool MovieProxyModel::filterDuplicates() const
{
    return m_filterDuplicates;
}

void MovieProxyModel::setFilterDuplicates(bool filterDuplicates)
{
    m_filterDuplicates = filterDuplicates;
    invalidate();
}

/**
 * @brief Sets active filters
 */
void MovieProxyModel::setFilter(QVector<Filter*> filters, QString text)
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
