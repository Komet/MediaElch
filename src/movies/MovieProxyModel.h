#pragma once

#include "globals/Filter.h"

#include <QSortFilterProxyModel>

class MovieProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MovieProxyModel(QObject* parent = nullptr);
    void setFilter(QVector<Filter*> filters, QString text);
    void setSortBy(SortBy sortBy);

    bool filterDuplicates() const;
    void setFilterDuplicates(bool filterDuplicates);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    /// \brief Sort function for the movie model. Sorts movies by name and new files to top per default.
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    QVector<Filter*> m_filters;
    QString m_filterText;
    SortBy m_sortBy;
    bool m_filterDuplicates;
};
