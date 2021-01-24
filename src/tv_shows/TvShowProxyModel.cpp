#include "TvShowProxyModel.h"

#include "globals/Globals.h"
#include "globals/Manager.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/SeasonModelItem.h"

TvShowProxyModel::TvShowProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
}

/**
 * \brief Checks if a row accepts the filter. Checks the first column of our model (TV Show name)
 */
/*
bool TvShowProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    QVector<TvShow*> shows = Manager::instance()->tvShowModel()->tvShows();
    if (sourceRow < 0 || sourceRow >= shows.count())
        return true;

    TvShow *show = shows.at(sourceRow);
    for (Filter *filter: m_filters) {
        if (!filter->accepts(show))
            return false;
    }

    return true;
}
*/

bool TvShowProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (filterAcceptsRowItself(source_row, source_parent)) {
        return true;
    }

    // accept if any of the parents is accepted on it's own merits
    QModelIndex parent = source_parent;
    while (parent.isValid()) {
        if (filterAcceptsRowItself(parent.row(), parent.parent())) {
            return true;
        }
        parent = parent.parent();
    }

    // accept if any of the children is accepted on it's own merits
    return hasAcceptedChildren(source_row, source_parent);
}

bool TvShowProxyModel::filterAcceptsRowItself(int sourceRow, const QModelIndex& sourceParent) const
{
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool TvShowProxyModel::hasAcceptedChildren(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex item = sourceModel()->index(source_row, 0, source_parent);
    if (!item.isValid()) {
        // qDebug() << "item invalid" << source_parent << source_row;
        return false;
    }

    // check if there are children
    int childCount = item.model()->rowCount(item);
    if (childCount == 0) {
        return false;
    }

    for (int i = 0; i < childCount; ++i) {
        if (filterAcceptsRowItself(i, item)) {
            return true;
        }
        // recursive call -> NOTICE that this is depth-first searching, you're probably better off with breadth first
        // search...
        if (hasAcceptedChildren(i, item)) {
            return true;
        }
    }

    return false;
}

/// \brief Sort function for the TV show model. Sorts TV shows by name.
bool TvShowProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    auto* model = dynamic_cast<TvShowModel*>(sourceModel());
    TvShowBaseModelItem& leftItem = model->getItem(left);
    TvShowBaseModelItem& rightItem = model->getItem(right);

    if (leftItem.type() == rightItem.type() && leftItem.type() == TvShowType::Season) {
        // todo: remove dynamic cast
        return dynamic_cast<SeasonModelItem*>(&leftItem)->seasonNumber()
               < dynamic_cast<SeasonModelItem*>(&rightItem)->seasonNumber();
    }

    if (leftItem.type() == rightItem.type() && leftItem.type() == TvShowType::Episode) {
        return dynamic_cast<EpisodeModelItem*>(&leftItem)->tvShowEpisode()->episodeNumber()
               < dynamic_cast<EpisodeModelItem*>(&rightItem)->tvShowEpisode()->episodeNumber();
    }

    if (leftItem.type() == rightItem.type() && leftItem.type() == TvShowType::TvShow) {
        bool leftNew = !leftItem.tvShow()->infoLoaded() || leftItem.tvShow()->hasNewEpisodes();
        bool rightNew = !rightItem.tvShow()->infoLoaded() || rightItem.tvShow()->hasNewEpisodes();
        if (leftNew && !rightNew) {
            return true;
        }
        if (!leftNew && rightNew) {
            return false;
        }
    }

    return (
        QString::localeAwareCompare(sourceModel()->data(left).toString(), sourceModel()->data(right).toString()) < 0);
}

void TvShowProxyModel::setFilter(QVector<Filter*> filters, QString text)
{
    m_filters = std::move(filters);
    m_filterText = std::move(text);
}
