#pragma once

#include "data/Actor.h"

#include <QAbstractTableModel>

class Actors;

class ActorModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ActorRoles
    {
        ActorRole = Qt::UserRole + 1,
        ImageRole = Qt::UserRole + 2
    };

    enum Columns
    {
        NameColumn = 0,
        RoleColumn = 1
    };

public:
    ActorModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {}
    ~ActorModel() override = default;

    void setActors(Actors* actors);
    void addActor(Actor actor);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool removeRows(int row, int count, const QModelIndex& parent = {}) override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    Actors* m_actors = nullptr;
};
