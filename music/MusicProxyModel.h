#ifndef MUSICPROXYMODEL_H
#define MUSICPROXYMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

class MusicProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MusicProxyModel(QObject *parent = 0);
    ~MusicProxyModel();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // MUSICPROXYMODEL_H
