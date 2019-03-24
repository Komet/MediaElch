#include "music/MusicProxyModel.h"

#include "globals/Globals.h"

MusicProxyModel::MusicProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

MusicProxyModel::~MusicProxyModel() = default;

bool MusicProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QString leftTitle = sourceModel()->data(left).toString();
    QString rightTitle = sourceModel()->data(right).toString();

    bool leftIsNew = sourceModel()->data(left, MusicRoles::IsNew).toBool();
    bool rightIsNew = sourceModel()->data(right, MusicRoles::IsNew).toBool();

    if (leftIsNew && !rightIsNew) {
        return true;
    }
    if (rightIsNew && !leftIsNew) {
        return false;
    }

    bool ret = (QString::localeAwareCompare(leftTitle, rightTitle) < 0);
    return ret;
}

bool MusicProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent)) {
        return true;
    }

    QModelIndex parent = sourceParent;
    while (parent.isValid()) {
        if (QSortFilterProxyModel::filterAcceptsRow(parent.row(), parent.parent())) {
            return true;
        }
        parent = parent.parent();
    }

    return hasAcceptedChildren(sourceRow, sourceParent);
}

void MusicProxyModel::setFilter(QVector<Filter*> filters, QString text)
{
    m_filters = filters;
    m_filterText = text;
}

bool MusicProxyModel::hasAcceptedChildren(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!index.isValid()) {
        return false;
    }

    if (index.model()->rowCount(index) == 0) {
        return false;
    }

    for (int i = 0, n = index.model()->rowCount(index); i < n; ++i) {
        if (QSortFilterProxyModel::filterAcceptsRow(i, index)) {
            return true;
        }
    }

    return false;
}
