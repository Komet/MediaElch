#include "ImageProxyModel.h"

ImageProxyModel::ImageProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{

}

bool ImageProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)
    return !sourceModel()->data(createIndex(sourceRow, 0), Qt::UserRole+2).toBool();
}
