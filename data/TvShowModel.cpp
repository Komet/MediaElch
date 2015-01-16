#include <QtGui>
#include <QDebug>
#include <QPainter>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "TvShowModel.h"
#include "TvShowModelItem.h"

/**
 * @brief TvShowModel::TvShowModel
 * @param parent
 */
TvShowModel::TvShowModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new TvShowModelItem(0);
    m_newIcon = QIcon(":/img/star_blue.png");
    m_syncIcon = QIcon(":/img/reload_orange.png");
    m_missingIcon = QIcon(":/img/missing.png");
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
        return Helper::instance()->appendArticle(item->data(0).toString());
    } else if (role == Qt::DecorationRole) {
        // new episodes or sync needed
        if (item->data(3).toBool())
            return m_newIcon;
        else if (item->data(4).toBool())
            return m_syncIcon;
        else if (item->type() == TypeSeason && item->tvShow()->hasDummyEpisodes(item->seasonNumber()))
            return m_missingIcon;
    } else if (role == TvShowRoles::Type) {
        return item->type();
    } else if (role == TvShowRoles::EpisodeCount && item->type() == TypeTvShow) {
        return item->data(1);
    } else if (role == Qt::ForegroundRole) {
        if (item->data(2).toBool())
            return QColor(255, 0, 0);
        if (item->type() == TypeEpisode && item->tvShowEpisode()->isDummy())
            return QColor(150, 150, 150);
        if (item->type() == TypeSeason && item->tvShow()->isDummySeason(item->seasonNumber()))
            return QColor(150, 150, 150);
    } else if (role == Qt::FontRole) {
        QFont font;
        if (!item->season().isEmpty())
            font.setBold(true);
        if (item->data(2).toBool())
            font.setItalic(true);
        return font;
    } else if (role == TvShowRoles::HasChanged) {
        return item->data(2);
    } else if (role == TvShowRoles::IsNew) {
        return item->data(3);
    } else if (role == TvShowRoles::SyncNeeded) {
        return item->data(4);
    } else if (role == TvShowRoles::HasBanner) {
        return item->data(101);
    } else if (role == TvShowRoles::HasPoster) {
        return item->data(102);
    } else if (role == TvShowRoles::HasExtraFanart) {
        return item->data(103);
    } else if (role == TvShowRoles::HasFanart) {
        return item->data(104);
    } else if (role == TvShowRoles::HasLogo) {
        return item->data(105);
    } else if (role == TvShowRoles::HasThumb) {
        return item->data(106);
    } else if (role == TvShowRoles::HasClearArt) {
        return item->data(107);
    } else if (role == TvShowRoles::HasCharacterArt) {
        return item->data(108);
    } else if (role == TvShowRoles::MissingEpisodes) {
        return item->data(109);
    } else if (role == TvShowRoles::LogoPath) {
        return item->data(110);
    } else if (role == TvShowRoles::FilePath && item->type() == TypeEpisode) {
        if (!item->tvShowEpisode()->files().isEmpty())
            return item->tvShowEpisode()->files().first();
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
int TvShowModel::hasNewShowOrEpisode()
{
    int newShows = 0;
    foreach (TvShow *show, tvShows()) {
        if (!show->infoLoaded())
            newShows++;
        foreach (TvShowEpisode *episode, show->episodes()) {
            if (!episode->infoLoaded())
                newShows++;
        }
    }
    return newShows;
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
