#ifndef CONCERTPROXYMODEL_H
#define CONCERTPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "globals/Filter.h"

/**
 * @brief The ConcertProxyModel class
 */
class ConcertProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ConcertProxyModel(QObject *parent = nullptr);
    void setFilter(QList<Filter *> filters, QString text);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    QList<Filter *> m_filters;
    QString m_filterText;
};

#endif // CONCERTPROXYMODEL_H
