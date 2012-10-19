#include "MovieProxyModel.h"

#include <QDebug>
#include "globals/Globals.h"
#include "globals/Manager.h"

/**
 * @brief MovieProxyModel::MovieProxyModel
 * @param parent
 */
MovieProxyModel::MovieProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
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
    if (sourceModel()->data(left, Qt::UserRole+1).toBool() && !sourceModel()->data(right, Qt::UserRole+1).toBool() )
        return true;
    if (!sourceModel()->data(left, Qt::UserRole+1).toBool() && sourceModel()->data(right, Qt::UserRole+1).toBool() )
        return false;
    int cmp = QString::compare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString());
    return !(cmp < 0);
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
