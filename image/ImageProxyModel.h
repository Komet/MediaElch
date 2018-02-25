#ifndef IMAGEPROXYMODEL_H
#define IMAGEPROXYMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

class ImageProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ImageProxyModel(QObject *parent = nullptr);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // IMAGEPROXYMODEL_H
