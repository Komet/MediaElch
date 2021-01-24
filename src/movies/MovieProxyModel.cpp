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
 * \brief Checks if a row accepts the filter. Checks the first two "columns" of our model (Movie name and folder name)
 * \return Filter is accepted or not
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

bool MovieProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    const QString leftTitle = sourceModel()->data(left, MovieModel::SortTitleRole).toString();
    const QString rightTitle = sourceModel()->data(right, MovieModel::SortTitleRole).toString();
    const int cmp = QString::localeAwareCompare(leftTitle, rightTitle);

    switch (m_sortBy) {
    case SortBy::Name: return (cmp < 0);

    case SortBy::Added:
        return sourceModel()->data(left, MovieModel::FileLastModifiedRole).toDateTime()
               >= sourceModel()->data(right, MovieModel::FileLastModifiedRole).toDateTime();

    case SortBy::Seen:
        if (sourceModel()->data(left, MovieModel::HasWatchedRole).toBool()
            && !sourceModel()->data(right, MovieModel::HasWatchedRole).toBool()) {
            return false;
        }
        if (!sourceModel()->data(left, MovieModel::HasWatchedRole).toBool()
            && sourceModel()->data(right, MovieModel::HasWatchedRole).toBool()) {
            return true;
        }
        // Otherwise sort by name because both are either seen or not.
        break;

    case SortBy::Year:
        if (sourceModel()->data(left, MovieModel::ReleasedRole).toDate().year()
            != sourceModel()->data(right, MovieModel::ReleasedRole).toDate().year()) {
            return sourceModel()->data(left, MovieModel::ReleasedRole).toDate().year()
                   >= sourceModel()->data(right, MovieModel::ReleasedRole).toDate().year();
        }
        // Otherwise sort by name because both have the same year.
        break;

    case SortBy::New:
        if (sourceModel()->data(left, MovieModel::InfoLoadedRole).toBool()
            && !sourceModel()->data(right, MovieModel::InfoLoadedRole).toBool()) {
            return false;
        }
        if (!sourceModel()->data(left, MovieModel::InfoLoadedRole).toBool()
            && sourceModel()->data(right, MovieModel::InfoLoadedRole).toBool()) {
            return true;
        }
        // Otherwise sort by name because both are new or not.
        break;
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

void MovieProxyModel::setFilter(QVector<Filter*> filters, QString text)
{
    m_filters = std::move(filters);
    m_filterText = std::move(text);
    invalidate();
}

void MovieProxyModel::setSortBy(SortBy sortBy)
{
    m_sortBy = sortBy;
    invalidate();
    sort(0, Qt::AscendingOrder);
}
