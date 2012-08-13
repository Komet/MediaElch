#include "MovieModel.h"

#include <QPainter>

MovieModel::MovieModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[FileNameRole] = "file";
    setRoleNames(roles);
}

void MovieModel::addMovie(Movie *movie)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_movies.append(movie);
    endInsertRows();
    connect(movie, SIGNAL(sigChanged(Movie*)), this, SLOT(onMovieChanged(Movie*)), Qt::UniqueConnection);
}

void MovieModel::onMovieChanged(Movie *movie)
{
    QModelIndex index = createIndex(m_movies.indexOf(movie), 0);
    emit dataChanged(index, index);
}

Movie *MovieModel::movie(int row)
{
    return m_movies.at(row);
}

int MovieModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_movies.size();
}

int MovieModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    // return roleNames().size();
    return 1;
}

QVariant MovieModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_movies.count())
        return QVariant();

    if (role == Qt::UserRole)
        return index.row();

    Movie *movie = m_movies[index.row()];
    if (index.column() == 0 && role == Qt::DisplayRole) {
        return movie->name();
    } else if (index.column() == 0 && role == Qt::ToolTipRole) {
        if (movie->files().size() == 0)
            return QVariant();
        return movie->files().at(0);
    } else if (index.column() == 1 && role == Qt::DisplayRole) {
        return movie->folderName();
    } else if (role == Qt::UserRole+1) {
        return movie->infoLoaded();
    } else if (role == Qt::UserRole+2) {
        return movie->hasChanged();
    } else if (role == Qt::ForegroundRole) {
        if (movie->hasChanged())
            return QColor(255, 0, 0);
    } else if (role == Qt::FontRole) {
        if (movie->hasChanged()) {
            QFont font;
            font.setItalic(true);
            return font;
        }
    }
    return QVariant();
}

QModelIndex MovieModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

QModelIndex MovieModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

void MovieModel::clear()
{
    if (m_movies.size() == 0)
        return;
    beginRemoveRows(QModelIndex(), 0, m_movies.size()-1);
    foreach (Movie *movie, m_movies)
        delete movie;
    m_movies.clear();
    endRemoveRows();
}

QList<Movie*> MovieModel::movies()
{
    return m_movies;
}
