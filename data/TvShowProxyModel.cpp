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
    Q_UNUSED(sourceParent);

    TvShow *show = Manager::instance()->tvShowModel()->tvShows().at(sourceRow);
    foreach (Filter *filter, m_filters) {
        if (!filter->accepts(show))
            return false;
    }

    return true;
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

/**
 * @brief Sets active filters
 * @param filters
 * @param text
 */
void TvShowProxyModel::setFilter(QList<Filter*> filters, QString text)
{
    m_filters = filters;
    m_filterText = text;
}
