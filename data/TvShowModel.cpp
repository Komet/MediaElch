#include <QtGui>

#include "globals/Globals.h"
#include "globals/Manager.h"
#include "TvShowModel.h"
#include "TvShowModelItem.h"
#include <QDebug>

/**
 * @brief TvShowModel::TvShowModel
 * @param parent
 */
TvShowModel::TvShowModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new TvShowModelItem(0);
}

/**
 * @brief TvShowModel::~TvShowModel
 */
TvShowModel::~TvShowModel()
{
    delete m_rootItem;
}

/**
 * @brief TvShowModel::columnCount
 * @param parent
 * @return Column count
 */
int TvShowModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_rootItem->columnCount();
}

/**
 * @brief TvShowModel::data
 * @param index
 * @param role
 * @return
 */
QVariant TvShowModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TvShowModelItem *item = getItem(index);
    if (role == Qt::DisplayRole) {
        return item->data(0);
    } else if (role == TvShowRoles::Type) {
        return item->type();
    } else if (role == TvShowRoles::EpisodeCount && item->type() == TypeTvShow) {
        return item->data(1);
    } else if (role == Qt::ForegroundRole) {
        if (item->data(2).toBool())
            return QColor(255, 0, 0);
    } else if (role == Qt::FontRole) {
        if (item->data(2).toBool()) {
            QFont font;
            font.setItalic(true);
            return font;
        }
    } else if (role == TvShowRoles::HasChanged) {
        return item->data(2);
    } else if (role == TvShowRoles::IsNew) {
        return item->data(3);
    } else if (role == TvShowRoles::SyncNeeded) {
        return item->data(4);
    }
    return QVariant();
}

/**
 * @brief TvShowModel::getItem
 * @param index
 * @return
 */
TvShowModelItem *TvShowModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TvShowModelItem *item = static_cast<TvShowModelItem*>(index.internalPointer());
        if (item) return item;
    }
    return m_rootItem;
}

/**
 * @brief TvShowModel::index
 * @param row
 * @param column
 * @param parent
 * @return
 */
QModelIndex TvShowModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TvShowModelItem *parentItem = getItem(parent);

    TvShowModelItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

/**
 * @brief TvShowModel::appendChild
 * @param show
 * @return
 */
TvShowModelItem *TvShowModel::appendChild(TvShow *show)
{
    TvShowModelItem *parentItem = m_rootItem;
    beginInsertRows(QModelIndex(), parentItem->childCount(), parentItem->childCount());
    TvShowModelItem *item = parentItem->appendChild(show);
    endInsertRows();
    connect(item, SIGNAL(sigChanged(TvShowModelItem*,TvShowModelItem*,TvShowModelItem*)), this, SLOT(onSigChanged(TvShowModelItem*,TvShowModelItem*,TvShowModelItem*)));
    connect(show, SIGNAL(sigChanged(TvShow*)), this, SLOT(onShowChanged(TvShow*)));
    return item;
}

/**
 * @brief TvShowModel::parent
 * @param index
 * @return
 */
QModelIndex TvShowModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TvShowModelItem *childItem = getItem(index);
    TvShowModelItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

/**
 * @brief TvShowModel::removeRows
 * @param position
 * @param rows
 * @param parent
 * @return
 */
bool TvShowModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TvShowModelItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

/**
 * @brief TvShowModel::rowCount
 * @param parent
 * @return Row count
 */
int TvShowModel::rowCount(const QModelIndex &parent) const
{
    TvShowModelItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

/**
 * @brief Removes all children
 */
void TvShowModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, m_rootItem->childCount());
    m_rootItem->removeChildren(0, m_rootItem->childCount());
    endRemoveRows();
}

/**
 * @brief TvShowModel::onSigChanged
 * @param showItem
 * @param seasonItem
 * @param episodeItem
 */
void TvShowModel::onSigChanged(TvShowModelItem *showItem, TvShowModelItem *seasonItem, TvShowModelItem *episodeItem)
{
    QModelIndex showIndex = this->index(showItem->childNumber(), 0);
    QModelIndex seasonIndex = this->index(seasonItem->childNumber(), 0, showIndex);
    QModelIndex index = this->index(episodeItem->childNumber(), 0, seasonIndex);
    emit dataChanged(index, index);
}

/**
 * @brief TvShowModel::onShowChanged
 * @param show
 */
void TvShowModel::onShowChanged(TvShow *show)
{
    QModelIndex index = this->index(show->modelItem()->childNumber(), 0);
    emit dataChanged(index, index);
}

/**
 * @brief TvShowModel::tvShows
 * @return
 */
QList<TvShow*> TvShowModel::tvShows()
{
    QList<TvShow*> shows;
    for (int i=0, n=m_rootItem->childCount() ; i<n ; ++i) {
        shows.append(m_rootItem->child(i)->tvShow());
    }
    return shows;
}

/**
 * @brief Checks if there are new shows or episodes (shows or episodes where infoLoaded is false)
 * @return True if there are new shows or episodes
 */
bool TvShowModel::hasNewShowOrEpisode()
{
    foreach (TvShow *show, tvShows()) {
        if (!show->infoLoaded())
            return true;
        foreach (TvShowEpisode *episode, show->episodes()) {
            if (!episode->infoLoaded())
                return true;
        }
    }
    return false;
}

/**
 * @brief TvShowModel::removeShow
 * @param show
 */
void TvShowModel::removeShow(TvShow *show)
{
    for (int i=0, n=m_rootItem->childCount() ; i<n ; ++i) {
        if (m_rootItem->child(i)->tvShow() == show) {
            removeRow(m_rootItem->child(i)->childNumber());
            return;
        }
    }
}
