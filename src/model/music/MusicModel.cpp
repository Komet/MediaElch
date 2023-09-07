#include "MusicModel.h"

#include "data/music/Album.h"
#include "globals/Helper.h"
#include "model/music/MusicModelRoles.h"

MusicModel::MusicModel(QObject* parent) : QAbstractItemModel(parent), m_rootItem{new MusicModelItem(nullptr)}
{
}

MusicModel::~MusicModel()
{
    delete m_rootItem;
}

int MusicModel::rowCount(const QModelIndex& parent) const
{
    MusicModelItem* parentItem = getItem(parent);
    if (parentItem != nullptr) {
        return parentItem->childCount();
    }
    return 0;
}

int MusicModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_rootItem->columnCount();
}

QVariant MusicModel::data(const QModelIndex& index, int role) const
{
    using namespace mediaelch;

    if (!index.isValid()) {
        return QVariant();
    }

    MusicModelItem* item = getItem(index);

    if (role == Qt::DisplayRole) {
        return helper::appendArticle(item->data(0).toString());
    }
    if (role == MusicRoles::Type) {
        return static_cast<int>(item->type());
    }
    if (role == MusicRoles::IsNew) {
        return item->data(role);
    }
    if (role == Qt::SizeHintRole) {
        return QSize(0, (item->type() == MusicType::Artist) ? 44 : 22);
    }
    if (role == MusicRoles::NumOfAlbums) {
        if (MusicType(item->data(MusicRoles::Type).toInt()) == MusicType::Artist) {
            return item->data(MusicRoles::NumOfAlbums);
        }
    }

    return QVariant();
}

MusicModelItem* MusicModel::getItem(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer() != nullptr) {
        return static_cast<MusicModelItem*>(index.internalPointer());
    }
    return m_rootItem;
}

QModelIndex MusicModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0) {
        return {};
    }

    MusicModelItem* parentItem = getItem(parent);
    MusicModelItem* childItem = parentItem->child(row);
    if (childItem != nullptr) {
        return createIndex(row, column, childItem);
    }
    return {};
}

MusicModelItem* MusicModel::appendChild(Artist* artist)
{
    beginInsertRows(QModelIndex(), m_rootItem->childCount(), m_rootItem->childCount());
    MusicModelItem* item = m_rootItem->appendChild(artist);
    endInsertRows();

    connect(item, &MusicModelItem::sigChanged, this, &MusicModel::onSigChanged, Qt::UniqueConnection);
    connect(artist, &Artist::sigChanged, this, &MusicModel::onArtistChanged, Qt::UniqueConnection);
    connect(
        artist->controller(), &ArtistController::sigSaved, this, &MusicModel::onArtistChanged, Qt::UniqueConnection);
    connect(item, &MusicModelItem::sigIntChanged, this, &MusicModel::onSigChanged, Qt::UniqueConnection);

    return item;
}

QModelIndex MusicModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return {};
    }

    MusicModelItem* childItem = getItem(index);
    MusicModelItem* parentItem = childItem->parent();

    if (parentItem == m_rootItem) {
        return {};
    }

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool MusicModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (count < 1 || row < 0 || (row + count) > rowCount(parent)) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    MusicModelItem* parentItem = getItem(parent);
    const bool success = parentItem->removeChildren(row, count);
    endRemoveRows();

    return success;
}

void MusicModel::clear()
{
    const int size = m_rootItem->childCount();
    if (size > 0) {
        beginRemoveRows(QModelIndex(), 0, size - 1);
        m_rootItem->removeChildren(0, size);
        endRemoveRows();
    }
}

void MusicModel::onSigChanged(MusicModelItem* artistItem, MusicModelItem* albumItem)
{
    QModelIndex artistIndex = index(artistItem->childNumber(), 0);
    QModelIndex albumIndex = index(albumItem->childNumber(), 0, artistIndex);
    emit dataChanged(albumIndex, albumIndex);
}

void MusicModel::onArtistChanged(Artist* artist)
{
    QModelIndex artistIndex = index(artist->modelItem()->childNumber(), 0);
    emit dataChanged(artistIndex, artistIndex);
}

QVector<Artist*> MusicModel::artists()
{
    QVector<Artist*> artists;
    for (int i = 0, n = m_rootItem->childCount(); i < n; ++i) {
        artists.append(m_rootItem->child(i)->artist());
    }
    return artists;
}

void MusicModel::removeArtist(Artist* artist)
{
    for (int i = 0, n = m_rootItem->childCount(); i < n; ++i) {
        if (m_rootItem->child(i)->artist() == artist) {
            removeRow(m_rootItem->child(i)->childNumber());
            return;
        }
    }
}

int MusicModel::hasNewArtistsOrAlbums()
{
    int newItems = 0;

    for (Artist* artist : artists()) {
        if (!artist->controller()->infoLoaded()) {
            newItems++;
        }
        for (int i = 0, n = artist->modelItem()->childNumber(); i < n; ++i) {
            if ((artist->modelItem()->child(i) != nullptr) && (artist->modelItem()->child(i)->album() != nullptr)
                && !artist->modelItem()->child(i)->album()->controller()->infoLoaded()) {
                newItems++;
            }
        }
    }

    return newItems;
}
