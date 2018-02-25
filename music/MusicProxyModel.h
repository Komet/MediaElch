#ifndef MUSICPROXYMODEL_H
#define MUSICPROXYMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

#include "../globals/Filter.h"

class MusicProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MusicProxyModel(QObject *parent = 0);
    ~MusicProxyModel();

    void setFilter(QList<Filter*> filters, QString text);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    QList<Filter*> m_filters;
    QString m_filterText;

    bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // MUSICPROXYMODEL_H
