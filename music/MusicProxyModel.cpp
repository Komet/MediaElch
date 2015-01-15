#include "MusicProxyModel.h"

#include "globals/Globals.h"

MusicProxyModel::MusicProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

MusicProxyModel::~MusicProxyModel()
{
}

bool MusicProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QString leftTitle = sourceModel()->data(left).toString();
    QString rightTitle = sourceModel()->data(right).toString();

    bool leftIsNew = sourceModel()->data(left, MusicRoles::IsNew).toBool();
    bool rightIsNew = sourceModel()->data(right, MusicRoles::IsNew).toBool();

    if (leftIsNew && !rightIsNew)
        return true;
    if (rightIsNew && !leftIsNew)
        return false;

    bool ret = (QString::localeAwareCompare(leftTitle, rightTitle) < 0);
    return ret;
}
