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

    m_icons.insert(TvShowRoles::HasPoster, QMap<bool, QIcon>());
    m_icons.insert(TvShowRoles::HasFanart, QMap<bool, QIcon>());
    m_icons.insert(TvShowRoles::HasExtraFanart, QMap<bool, QIcon>());
    m_icons.insert(TvShowRoles::HasThumb, QMap<bool, QIcon>());
    m_icons.insert(TvShowRoles::HasLogo, QMap<bool, QIcon>());
    m_icons.insert(TvShowRoles::HasClearArt, QMap<bool, QIcon>());
    m_icons.insert(TvShowRoles::HasCharacterArt, QMap<bool, QIcon>());
    m_icons.insert(TvShowRoles::HasBanner, QMap<bool, QIcon>());

    m_icons[TvShowRoles::HasPoster].insert(false, QIcon(":mediaStatus/poster/red"));
    m_icons[TvShowRoles::HasPoster].insert(true, QIcon(":mediaStatus/poster/green"));
    m_icons[TvShowRoles::HasFanart].insert(false, QIcon(":mediaStatus/fanart/red"));
    m_icons[TvShowRoles::HasFanart].insert(true, QIcon(":mediaStatus/fanart/green"));
    m_icons[TvShowRoles::HasExtraFanart].insert(false, QIcon(":mediaStatus/extraFanarts/red"));
    m_icons[TvShowRoles::HasExtraFanart].insert(true, QIcon(":mediaStatus/extraFanarts/green"));
    m_icons[TvShowRoles::HasThumb].insert(false, QIcon(":mediaStatus/thumb/red"));
    m_icons[TvShowRoles::HasThumb].insert(true, QIcon(":mediaStatus/thumb/green"));
    m_icons[TvShowRoles::HasLogo].insert(false, QIcon(":mediaStatus/logo/red"));
    m_icons[TvShowRoles::HasLogo].insert(true, QIcon(":mediaStatus/logo/green"));
    m_icons[TvShowRoles::HasClearArt].insert(false, QIcon(":mediaStatus/clearart/red"));
    m_icons[TvShowRoles::HasClearArt].insert(true, QIcon(":mediaStatus/clearart/green"));
    m_icons[TvShowRoles::HasCharacterArt].insert(false, QIcon(":mediaStatus/actors/red"));
    m_icons[TvShowRoles::HasCharacterArt].insert(true, QIcon(":mediaStatus/actors/green"));
    m_icons[TvShowRoles::HasBanner].insert(false, QIcon(":mediaStatus/banner/red"));
    m_icons[TvShowRoles::HasBanner].insert(true, QIcon(":mediaStatus/banner/green"));
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
    if (!parent.isValid())
        return 9;
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

    if (index.column() != 0) {
        if (role == Qt::DecorationRole) {
            switch (index.column()) {
            case 1:
                return m_icons.value(TvShowRoles::HasPoster).value(item->data(102).toBool());
            case 2:
                return m_icons.value(TvShowRoles::HasFanart).value(item->data(104).toBool());
            case 3:
                return m_icons.value(TvShowRoles::HasExtraFanart).value(item->data(103).toBool());
            case 4:
                return m_icons.value(TvShowRoles::HasThumb).value(item->data(106).toBool());
            case 5:
                return m_icons.value(TvShowRoles::HasLogo).value(item->data(105).toBool());
            case 6:
                return m_icons.value(TvShowRoles::HasClearArt).value(item->data(107).toBool());
            case 7:
                return m_icons.value(TvShowRoles::HasCharacterArt).value(item->data(108).toBool());
            case 8:
                return m_icons.value(TvShowRoles::HasBanner).value(item->data(101).toBool());
            }
        } else if (role == Qt::ToolTipRole) {
            switch (index.column()) {
            case 1:
                return tr("Poster");
            case 2:
                return tr("Fanart");
            case 3:
                return tr("Extra Fanarts");
            case 4:
                return tr("Thumb");
            case 5:
                return tr("Logo");
            case 6:
                return tr("Clear Art");
            case 7:
                return tr("Character Art");
            case 8:
                return tr("Banner");
            }
        }
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return Helper::instance()->appendArticle(item->data(0).toString());
    } else if (role == Qt::FontRole) {
        QFont font;
        if (item->data(2).toBool())
            font.setItalic(true);
        if (item->type() == TypeTvShow || item->type() == TypeSeason)
            font.setBold(true);

        if (item->type() == TypeSeason || item->type() == TypeEpisode) {
#ifdef Q_OS_MAC
            font.setPointSize(font.pointSize()-2);
#endif
        }
        return font;
    } else if (role == Qt::SizeHintRole) {
        return QSize(0, (item->type() == TypeTvShow) ? 44 : (item->type() == TypeSeason) ? 26 : 22);
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
        return QColor(17, 51, 80);
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
    } else if (role == TvShowRoles::SelectionForeground) {
        return QColor(255, 255, 255);
    } else if (role == TvShowRoles::FilePath && item->type() == TypeEpisode) {
        if (!item->tvShowEpisode()->files().isEmpty())
            return item->tvShowEpisode()->files().first();
    } else if (role == TvShowRoles::HasDummyEpisodes) {
        if (item->type() == TypeSeason && item->tvShow()->hasDummyEpisodes(item->seasonNumber()))
            return true;
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
