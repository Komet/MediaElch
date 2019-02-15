#pragma once

#include "globals/Filter.h"

#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>
#include <QVector>

class MusicProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit MusicProxyModel(QObject* parent = nullptr);
    ~MusicProxyModel() override;

    void setFilter(QVector<Filter*> filters, QString text);

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QVector<Filter*> m_filters;
    QString m_filterText;

    bool hasAcceptedChildren(int sourceRow, const QModelIndex& sourceParent) const;
};
