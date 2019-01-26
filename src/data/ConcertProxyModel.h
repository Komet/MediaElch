#pragma once

#include <QSortFilterProxyModel>

class Filter;

class ConcertProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ConcertProxyModel(QObject *parent = nullptr);
    void setFilter(QList<Filter *> filters, QString text);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QList<Filter *> m_filters;
    QString m_filterText;
};
