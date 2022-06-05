#include "data/ActorModel.h"

#include "data/Actor.h"
#include "movies/Movie.h"

void ActorModel::setActors(Actors* actors)
{
    beginResetModel();
    m_actors = actors;
    endResetModel();
}

void ActorModel::addActor(Actor actor)
{
    if (m_actors == nullptr) {
        return;
    }
    QModelIndex root{};
    beginInsertRows(root, rowCount(), rowCount());
    m_actors->addActor(std::move(actor));
    endInsertRows();
}

int ActorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || m_actors == nullptr) {
        // Root has an invalid model index.
        return 0;
    }
    return qsizetype_to_int(m_actors->actors().size());
}

int ActorModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        // Root has an invalid model index.
        return 0;
    }
    return 2;
}

QVariant ActorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || m_actors == nullptr) {
        return {};
    }

    if (index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }

    Actor* actor = m_actors->actors().at(index.row());

    switch (role) {
    case ActorRoles::ActorRole: return QVariant::fromValue(actor);

    case ActorRoles::ImageRole: {
        return actor->image;
        break;
    }

    case Qt::DisplayRole:
    case Qt::EditRole: {
        switch (index.column()) {
        case Columns::NameColumn: return actor->name;
        case Columns::RoleColumn: return actor->role;
        }
        break;
    }
    }

    return {};
}

QVariant ActorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Vertical) {
        return {};
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return {};
    }

    switch (section) {
    case Columns::NameColumn: return tr("Actor");
    case Columns::RoleColumn: return tr("Role");
    }

    return {};
}

bool ActorModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || m_actors == nullptr) {
        // non-root element not possible in table view
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    QVector<Actor*> actors = m_actors->actors();
    for (int i = row; i < row + count; ++i) {
        m_actors->removeActor(actors[i]);
    }
    endRemoveRows();

    return true;
}

bool ActorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || m_actors == nullptr) {
        // root element can't be edited
        return false;
    }
    if (role != Qt::EditRole) {
        return false;
    }

    Actor* actor = m_actors->actors().at(index.row());
    switch (index.column()) {
    case Columns::NameColumn: actor->name = value.toString(); break;
    case Columns::RoleColumn: actor->role = value.toString(); break;
    }

    emit dataChanged(index, index, {role});

    return true;
}

Qt::ItemFlags ActorModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.isValid()) {
        f |= Qt::ItemIsEditable;
    }
    return f;
}
