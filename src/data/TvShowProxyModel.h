#pragma once

#include "globals/Filter.h"

#include <QSortFilterProxyModel>

/**
 * @brief The TvShowProxyModel class
 */
class TvShowProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TvShowProxyModel(QObject *parent = nullptr);
    void setFilter(QList<Filter *> filters, QString text);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool filterAcceptsRowItself(int sourceRow, const QModelIndex &sourceParent) const;
    bool hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QList<Filter *> m_filters;
    QString m_filterText;
};
