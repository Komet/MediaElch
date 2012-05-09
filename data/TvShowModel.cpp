#include <QtGui>

#include "Globals.h"
#include "Manager.h"
#include "TvShowModel.h"
#include "TvShowModelItem.h"
#include <QDebug>

TvShowModel::TvShowModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new TvShowModelItem(0);
}

TvShowModel::~TvShowModel()
{
    delete m_rootItem;
}

int TvShowModel::columnCount(const QModelIndex & /* parent */) const
{
    return m_rootItem->columnCount();
}

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
    }
    return QVariant();
}

TvShowModelItem *TvShowModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TvShowModelItem *item = static_cast<TvShowModelItem*>(index.internalPointer());
        if (item) return item;
    }
    return m_rootItem;
}

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

TvShowModelItem *TvShowModel::appendChild(TvShow *show)
{
    TvShowModelItem *parentItem = m_rootItem;
    //beginInsertRows(QModelIndex(), parentItem->childCount(), parentItem->childCount());
    TvShowModelItem *item = parentItem->appendChild(show);
    //endInsertRows();
    connect(item, SIGNAL(sigChanged(TvShowModelItem*,TvShowModelItem*)), this, SLOT(onSigChanged(TvShowModelItem*,TvShowModelItem*)));
    connect(show, SIGNAL(sigChanged(TvShow*)), this, SLOT(onShowChanged(TvShow*)));
    return item;
}

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

bool TvShowModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TvShowModelItem *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int TvShowModel::rowCount(const QModelIndex &parent) const
{
    TvShowModelItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

void TvShowModel::clear()
{
    m_rootItem->removeChildren(0, m_rootItem->childCount());
}

void TvShowModel::onSigChanged(TvShowModelItem *showItem, TvShowModelItem *episodeItem)
{
    QModelIndex parentIndex = this->index(showItem->childNumber(), 0);
    QModelIndex index = this->index(episodeItem->childNumber(), 0, parentIndex);
    emit dataChanged(index, index);
}

void TvShowModel::onShowChanged(TvShow *show)
{
    QModelIndex index = this->index(show->modelItem()->childNumber(), 0);
    emit dataChanged(index, index);
}

QList<TvShow*> TvShowModel::tvShows()
{
    QList<TvShow*> shows;
    for (int i=0, n=m_rootItem->childCount() ; i<n ; ++i) {
        shows.append(m_rootItem->child(i)->tvShow());
    }
    return shows;
}
