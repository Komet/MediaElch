#ifndef MOVIEPROXYMODEL_H
#define MOVIEPROXYMODEL_H

#include <QSortFilterProxyModel>

/**
 * @brief The MovieProxyModel class
 */
class MovieProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MovieProxyModel(QObject *parent = 0);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // MOVIEPROXYMODEL_H
