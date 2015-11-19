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
/*
bool TvShowProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    QList<TvShow*> shows = Manager::instance()->tvShowModel()->tvShows();
    if (sourceRow < 0 || sourceRow >= shows.count())
        return true;

    TvShow *show = shows.at(sourceRow);
    foreach (Filter *filter, m_filters) {
        if (!filter->accepts(show))
            return false;
    }

    return true;
}
*/

bool TvShowProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (filterAcceptsRowItself(source_row, source_parent))
        return true;

    //accept if any of the parents is accepted on it's own merits
    QModelIndex parent = source_parent;
    while (parent.isValid()) {
        if (filterAcceptsRowItself(parent.row(), parent.parent()))
            return true;
        parent = parent.parent();
    }

    //accept if any of the children is accepted on it's own merits
    if (hasAcceptedChildren(source_row, source_parent)) {
        return true;
    }

    return false;
}

bool TvShowProxyModel::filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const
{
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool TvShowProxyModel::hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex item = sourceModel()->index(source_row, 0, source_parent);
    if (!item.isValid()) {
        //qDebug() << "item invalid" << source_parent << source_row;
        return false;
    }

    //check if there are children
    int childCount = item.model()->rowCount(item);
    if (childCount == 0)
        return false;

    for (int i = 0; i < childCount; ++i) {
        if (filterAcceptsRowItself(i, item))
            return true;
        //recursive call -> NOTICE that this is depth-first searching, you're probably better off with breadth first search...
        if (hasAcceptedChildren(i, item))
            return true;
    }

    return false;
}


/**
 * @brief Sort function for the tv show model. Sorts tv shows by name.
 * @param left
 * @param right
 * @return
 */
bool TvShowProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    TvShowModel *model = static_cast<TvShowModel*>(sourceModel());
    TvShowModelItem *leftItem = model->getItem(left);
    TvShowModelItem *rightItem = model->getItem(right);

    if (leftItem->type() == rightItem->type() && leftItem->type() == TypeSeason)
        return leftItem->seasonNumber() < rightItem->seasonNumber();

    if (leftItem->type() == rightItem->type() && leftItem->type() == TypeEpisode)
        return leftItem->tvShowEpisode()->episode() < rightItem->tvShowEpisode()->episode();

    if (leftItem->type() == rightItem->type() && leftItem->type() == TypeTvShow) {
        bool leftNew = !leftItem->tvShow()->infoLoaded() || leftItem->tvShow()->hasNewEpisodes();
        bool rightNew = !rightItem->tvShow()->infoLoaded() || rightItem->tvShow()->hasNewEpisodes();
        if (leftNew && !rightNew)
            return true;
        if (!leftNew && rightNew)
            return false;
    }

    return (QString::localeAwareCompare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString()) < 0);
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
