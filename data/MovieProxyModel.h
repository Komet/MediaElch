#ifndef MOVIEPROXYMODEL_H
#define MOVIEPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "globals/Filter.h"

/**
 * @brief The MovieProxyModel class
 */
class MovieProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MovieProxyModel(QObject *parent = 0);
    void setFilter(QList<Filter*> filters, QString text);
    void setSortBy(SortBy sortBy);
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
private:
    QList<Filter*> m_filters;
    QString m_filterText;
    SortBy m_sortBy;
};

#endif // MOVIEPROXYMODEL_H
