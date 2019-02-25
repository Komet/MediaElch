#include <QDebug>
#include <QPainter>
#include <QtGui>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "tvShows/TvShowModel.h"
#include "tvShows/model/EpisodeModelItem.h"
#include "tvShows/model/SeasonModelItem.h"
#include "tvShows/model/TvShowModelItem.h"

TvShowModel::TvShowModel(QObject* parent) :
    QAbstractItemModel(parent),
    m_newIcon{QIcon(":/img/star_blue.png")},
    m_syncIcon{QIcon(":/img/reload_orange.png")},
    m_missingIcon{QIcon(":/img/missing.png")}
{
    m_icons.insert(TvShowRoles::HasPoster, {});
    m_icons.insert(TvShowRoles::HasFanart, {});
    m_icons.insert(TvShowRoles::HasExtraFanart, {});
    m_icons.insert(TvShowRoles::HasThumb, {});
    m_icons.insert(TvShowRoles::HasLogo, {});
    m_icons.insert(TvShowRoles::HasClearArt, {});
    m_icons.insert(TvShowRoles::HasCharacterArt, {});
    m_icons.insert(TvShowRoles::HasBanner, {});

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

int TvShowModel::columnCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return m_icons.size() + 1; // each icon is a column + text
    }
    return m_rootItem.columnCount();
}

QVariant TvShowModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const TvShowBaseModelItem& item = getItem(index);

    if (index.column() != 0) { // column 0 => text
        if (role == Qt::DecorationRole) {
            switch (index.column()) {
            case 1: return m_icons.value(TvShowRoles::HasPoster).value(item.data(102).toBool());
            case 2: return m_icons.value(TvShowRoles::HasFanart).value(item.data(104).toBool());
            case 3: return m_icons.value(TvShowRoles::HasExtraFanart).value(item.data(103).toBool());
            case 4: return m_icons.value(TvShowRoles::HasThumb).value(item.data(106).toBool());
            case 5: return m_icons.value(TvShowRoles::HasLogo).value(item.data(105).toBool());
            case 6: return m_icons.value(TvShowRoles::HasClearArt).value(item.data(107).toBool());
            case 7: return m_icons.value(TvShowRoles::HasCharacterArt).value(item.data(108).toBool());
            case 8: return m_icons.value(TvShowRoles::HasBanner).value(item.data(101).toBool());
            }
        } else if (role == Qt::ToolTipRole) {
            switch (index.column()) {
            case 1: return tr("Poster");
            case 2: return tr("Fanart");
            case 3: return tr("Extra Fanarts");
            case 4: return tr("Thumb");
            case 5: return tr("Logo");
            case 6: return tr("Clear Art");
            case 7: return tr("Character Art");
            case 8: return tr("Banner");
            }
        }
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return Helper::instance()->appendArticle(item.data(0).toString());
    } else if (role == Qt::FontRole) {
        QFont font;
        if (item.data(2).toBool()) {
            font.setItalic(true);
        }
        if (item.type() == TvShowType::TvShow || item.type() == TvShowType::Season) {
            font.setBold(true);
        }

        if (item.type() == TvShowType::Season || item.type() == TvShowType::Episode) {
#ifdef Q_OS_MAC
            font.setPointSize(font.pointSize() - 2);
#endif
        }
        return font;
    } else if (role == Qt::SizeHintRole) {
        return QSize(0, (item.type() == TvShowType::TvShow) ? 44 : (item.type() == TvShowType::Season) ? 26 : 22);
    } else if (role == TvShowRoles::Type) {
        return static_cast<int>(item.type());
    } else if (role == TvShowRoles::EpisodeCount && item.type() == TvShowType::TvShow) {
        return item.data(1);
    } else if (role == Qt::ForegroundRole) {
        // hasChanged()
        if (item.data(2).toBool()) {
            return QColor(255, 0, 0);
        }
        if (item.type() == TvShowType::Episode) {
            auto* showEpisode = dynamic_cast<const EpisodeModelItem*>(&item);
            if (showEpisode->tvShowEpisode()->isDummy()) {
                return QColor(150, 150, 150);
            }
        }
        if (item.type() == TvShowType::Season) {
            auto* seasonModel = dynamic_cast<const SeasonModelItem*>(&item);
            if (item.tvShow()->isDummySeason(seasonModel->seasonNumber())) {
                return QColor(150, 150, 150);
            }
        }
        return QColor(17, 51, 80);
    }

    switch (role) {
    case TvShowRoles::HasChanged: return item.data(2);
    case TvShowRoles::IsNew: return item.data(3);
    case TvShowRoles::SyncNeeded: return item.data(4);
    case TvShowRoles::HasBanner: return item.data(101);
    case TvShowRoles::HasPoster: return item.data(102);
    case TvShowRoles::HasExtraFanart: return item.data(103);
    case TvShowRoles::HasFanart: return item.data(104);
    case TvShowRoles::HasLogo: return item.data(105);
    case TvShowRoles::HasThumb: return item.data(106);
    case TvShowRoles::HasClearArt: return item.data(107);
    case TvShowRoles::HasCharacterArt: return item.data(108);
    case TvShowRoles::MissingEpisodes: return item.data(109);
    case TvShowRoles::LogoPath: return item.data(110);
    case TvShowRoles::SelectionForeground: return QColor(255, 255, 255);
    case TvShowRoles::FilePath:
        if (item.type() == TvShowType::Episode) {
            auto* episode = dynamic_cast<const EpisodeModelItem*>(&item);
            if (!episode->tvShowEpisode()->files().isEmpty()) {
                return episode->tvShowEpisode()->files().first();
            }
        }
        if (item.type() == TvShowType::TvShow) {
            return item.tvShow()->dir();
        }
        break;
    case TvShowRoles::HasDummyEpisodes:
        if (item.type() == TvShowType::Season) {
            auto* season = dynamic_cast<const SeasonModelItem*>(&item);
            if (item.tvShow()->hasDummyEpisodes(season->seasonNumber())) {
                return true;
            }
        }
        break;
    }
    return QVariant();
}

const TvShowBaseModelItem& TvShowModel::getItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        auto* item = static_cast<TvShowBaseModelItem*>(index.internalPointer());
        if (item != nullptr) {
            return *item;
        }
    }
    return m_rootItem;
}

TvShowBaseModelItem& TvShowModel::getItem(const QModelIndex& index)
{
    if (index.isValid()) {
        auto* item = static_cast<TvShowBaseModelItem*>(index.internalPointer());
        if (item) {
            return *item;
        }
    }
    return m_rootItem;
}

QModelIndex TvShowModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0) {
        return QModelIndex{};
    }

    TvShowBaseModelItem* childItem = getItem(parent).child(row);
    if (childItem != nullptr) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex{};
}

TvShowModelItem* TvShowModel::appendChild(TvShow* show)
{
    const int size = m_rootItem.shows().size();

    beginInsertRows(QModelIndex{}, size, size);
    TvShowModelItem* item = m_rootItem.appendShow(show);
    endInsertRows();

    connect(item, &TvShowModelItem::sigChanged, this, &TvShowModel::onSigChanged);
    connect(show, &TvShow::sigChanged, this, &TvShowModel::onShowChanged);
    return item;
}

QModelIndex TvShowModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TvShowBaseModelItem* parentItem = getItem(index).parent();
    if (parentItem == nullptr || parentItem == &m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->indexInParent(), 0, parentItem);
}

bool TvShowModel::removeRows(int position, int rows, const QModelIndex& parent)
{
    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = getItem(parent).removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int TvShowModel::rowCount(const QModelIndex& parent) const
{
    return getItem(parent).childCount();
}

/// @brief Removes all children
void TvShowModel::clear()
{
    const int size = m_rootItem.shows().size();
    beginRemoveRows(QModelIndex(), 0, size);
    m_rootItem.removeChildren(0, size);
    endRemoveRows();
}

void TvShowModel::onSigChanged(TvShowModelItem* showItem, SeasonModelItem* seasonItem, EpisodeModelItem* episodeItem)
{
    const QModelIndex showIndex = index(showItem->indexInParent(), 0);
    const QModelIndex seasonIndex = index(seasonItem->indexInParent(), 0, showIndex);
    const QModelIndex modelIndex = index(episodeItem->indexInParent(), 0, seasonIndex);
    emit dataChanged(modelIndex, modelIndex);
}

void TvShowModel::onShowChanged(TvShow* show)
{
    const QModelIndex modelIndex = index(show->modelItem()->indexInParent(), 0);
    emit dataChanged(modelIndex, modelIndex);
}

QVector<TvShow*> TvShowModel::tvShows()
{
    QVector<TvShow*> shows;
    for (auto* modelShow : m_rootItem.shows()) {
        shows.push_back(modelShow->tvShow());
    }
    return shows;
}

/// @brief Checks if there are new shows or episodes (shows or episodes where infoLoaded is false).
/// @return True if there are new shows or episodes.
int TvShowModel::hasNewShowOrEpisode()
{
    int newShows = 0;
    for (TvShow* show : tvShows()) {
        if (!show->infoLoaded()) {
            ++newShows;
        }
        for (TvShowEpisode* episode : show->episodes()) {
            if (!episode->infoLoaded()) {
                ++newShows;
            }
        }
    }
    return newShows;
}

void TvShowModel::removeShow(TvShow* show)
{
    for (auto* showModel : m_rootItem.shows()) {
        if (showModel->tvShow() == show) {
            removeRow(showModel->indexInParent());
            return;
        }
    }
}
