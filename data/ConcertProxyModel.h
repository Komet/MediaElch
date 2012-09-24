#ifndef CONCERTPROXYMODEL_H
#define CONCERTPROXYMODEL_H

#include <QSortFilterProxyModel>

/**
 * @brief The ConcertProxyModel class
 */
class ConcertProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ConcertProxyModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // CONCERTPROXYMODEL_H
