#include "MusicModel.h"

#include "globals/Globals.h"
#include "globals/Helper.h"

MusicModel::MusicModel(QObject *parent) : QAbstractItemModel(parent)
{
    m_rootItem = new MusicModelItem(0);
}

MusicModel::~MusicModel()
{
    delete m_rootItem;
}

int MusicModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_rootItem->columnCount();
}

QVariant MusicModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    MusicModelItem *item = getItem(index);

    if (role == Qt::DisplayRole) {
        return Helper::instance()->appendArticle(item->data(0).toString());
    } else if (role == MusicRoles::Type) {
        return item->type();
    } else if (role == Qt::ForegroundRole) {
        if (item->data(2).toBool())
            return QColor(255, 0, 0);
    } else if (role == Qt::FontRole) {
        QFont font;
        if (item->data(2).toBool())
            font.setItalic(true);
        return font;
    }

    return QVariant();
}

MusicModelItem *MusicModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        MusicModelItem *item = static_cast<MusicModelItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_rootItem;
}

QModelIndex MusicModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    MusicModelItem *parentItem = getItem(parent);

    MusicModelItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

MusicModelItem *MusicModel::appendChild(Artist *artist)
{
    MusicModelItem *parentItem = m_rootItem;
    beginInsertRows(QModelIndex(), parentItem->childCount(), parentItem->childCount());
    MusicModelItem *item = parentItem->appendChild(artist);
    endInsertRows();
    connect(item, &MusicModelItem::sigChanged, this, &MusicModel::onSigChanged);
    connect(artist, &Artist::sigChanged, this, &MusicModel::onArtistChanged);
    return item;
}

QModelIndex MusicModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    MusicModelItem *childItem = getItem(index);
    MusicModelItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool MusicModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    MusicModelItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int MusicModel::rowCount(const QModelIndex &parent) const
{
    MusicModelItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

void MusicModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, m_rootItem->childCount());
    m_rootItem->removeChildren(0, m_rootItem->childCount());
    endRemoveRows();
}

void MusicModel::onSigChanged(MusicModelItem *artistItem, MusicModelItem *albumItem)
{
    QModelIndex artistIndex = this->index(artistItem->childNumber(), 0);
    QModelIndex index = this->index(albumItem->childNumber(), 0, artistIndex);
    emit dataChanged(index, index);
}

void MusicModel::onArtistChanged(Artist *artist)
{
    QModelIndex index = this->index(artist->modelItem()->childNumber(), 0);
    emit dataChanged(index, index);
}

QList<Artist*> MusicModel::artists()
{
    QList<Artist*> artists;
    for (int i=0, n=m_rootItem->childCount() ; i<n ; ++i) {
        artists.append(m_rootItem->child(i)->artist());
    }
    return artists;
}

void MusicModel::removeArtist(Artist *artist)
{
    for (int i=0, n=m_rootItem->childCount() ; i<n ; ++i) {
        if (m_rootItem->child(i)->artist() == artist) {
            removeRow(m_rootItem->child(i)->childNumber());
            return;
        }
    }
}
