#pragma once

#include "globals/Filter.h"

#include <QList>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>

class MusicProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MusicProxyModel(QObject *parent = nullptr);
    ~MusicProxyModel() override;

    void setFilter(QList<Filter *> filters, QString text);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QList<Filter *> m_filters;
    QString m_filterText;

    bool hasAcceptedChildren(int sourceRow, const QModelIndex &sourceParent) const;
};
