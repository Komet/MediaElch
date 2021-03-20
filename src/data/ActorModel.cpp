#include "data/ActorModel.h"

#include "data/Actor.h"
#include "movies/Movie.h"

void ActorModel::setMovie(Movie* movie)
{
    beginResetModel();
    m_movie = movie;
    endResetModel();
}

void ActorModel::addActorToMovie(Actor actor)
{
    if (m_movie == nullptr) {
        return;
    }
    QModelIndex root{};
    beginInsertRows(root, rowCount(), rowCount());
    m_movie->addActor(std::move(actor));
    m_movie->setChanged(true);
    endInsertRows();
}

int ActorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || m_movie == nullptr) {
        // Root has an invalid model index.
        return 0;
    }
    return m_movie->actors().size();
}

int ActorModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid() || m_movie == nullptr) {
        // Root has an invalid model index.
        return 0;
    }
    return 2;
}

QVariant ActorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || m_movie == nullptr) {
        return {};
    }

    if (index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }

    Actor* actor = m_movie->actors().at(index.row());

    switch (role) {
    case ActorRoles::ActorRole: return QVariant::fromValue(actor);

    case ActorRoles::ImageRole: {
        return actor->image;
        break;
    }

    case Qt::DisplayRole:
    case Qt::EditRole: {
        switch (index.column()) {
        case 0: return actor->name;
        case 1: return actor->role;
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

    if (role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
    case 0: return tr("Actor");
    case 1: return tr("Role");
    }

    return {};
}

bool ActorModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent.isValid() || m_movie == nullptr) {
        // non-root element not possible in table view
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    QVector<Actor*> actors = m_movie->actors();
    for (int i = row; i < row + count; ++i) {
        m_movie->removeActor(actors[i]);
    }
    m_movie->setChanged(true);
    endRemoveRows();

    return true;
}

bool ActorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || m_movie == nullptr) {
        // root element can't be edited
        return false;
    }
    if (role != Qt::EditRole) {
        return false;
    }

    Actor* actor = m_movie->actors().at(index.row());
    switch (index.column()) {
    case 0: actor->name = value.toString(); break;
    case 1: actor->role = value.toString(); break;
    }

    m_movie->setChanged(true);
    emit dataChanged(index, index, {role});

    return true;
}

Qt::ItemFlags ActorModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.isValid())
        f |= Qt::ItemIsEditable;
    return f;
}
