#ifndef TVSHOWPROXYMODEL_H
#define TVSHOWPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "globals/Filter.h"

/**
 * @brief The TvShowProxyModel class
 */
class TvShowProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TvShowProxyModel(QObject *parent = 0);
    void setFilter(QList<Filter*> filters, QString text);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const;
    bool hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    QList<Filter*> m_filters;
    QString m_filterText;
};

#endif // TVSHOWPROXYMODEL_H
