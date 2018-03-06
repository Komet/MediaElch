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
    explicit MovieProxyModel(QObject *parent = nullptr);
    void setFilter(QList<Filter *> filters, QString text);
    void setSortBy(SortBy sortBy);

    bool filterDuplicates() const;
    void setFilterDuplicates(bool filterDuplicates);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QList<Filter *> m_filters;
    QString m_filterText;
    SortBy m_sortBy;
    bool m_filterDuplicates;
};

#endif // MOVIEPROXYMODEL_H
