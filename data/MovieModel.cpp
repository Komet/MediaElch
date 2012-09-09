#include "MovieModel.h"

#include <QPainter>
#include "globals/Globals.h"

/**
 * @brief MovieModel::MovieModel
 * @param parent
 */
MovieModel::MovieModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[FileNameRole] = "file";
    setRoleNames(roles);
}

/**
 * @brief Adds a movie to the model
 * @param movie Movie to add
 */
void MovieModel::addMovie(Movie *movie)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_movies.append(movie);
    endInsertRows();
    connect(movie, SIGNAL(sigChanged(Movie*)), this, SLOT(onMovieChanged(Movie*)), Qt::UniqueConnection);
}

/**
 * @brief Called when a movies data has changed
 * Emits dataChanged
 * @param movie Movie which has changed
 */
void MovieModel::onMovieChanged(Movie *movie)
{
    QModelIndex index = createIndex(m_movies.indexOf(movie), 0);
    emit dataChanged(index, index);
}

/**
 * @brief Get a specific movie
 * @param row Row of the movie
 * @return Movie object
 */
Movie *MovieModel::movie(int row)
{
    return m_movies.at(row);
}

/**
 * @brief Returns the rowcount in our model. (=number of movies)
 * @param parent
 * @return Number of rows (=number of movies)
 */
int MovieModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_movies.size();
}

/**
 * @brief Get the column count of our model
 * @param parent
 * @return 1
 */
int MovieModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    // return roleNames().size();
    return 1;
}

/**
 * @brief Accesses items data
 * @param index Index of item
 * @param role Role
 * @return data
 */
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

/**
 * @brief Returns an empty modelindex because no item has a parent
 * @param child Childindex
 * @return Modelindex of the parent item
 */
QModelIndex MovieModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

/**
 * @brief Returns a modelindex for the given row and column
 * Parent is not used because our movie model uses only one column.
 * @param row Row of the item
 * @param column Column of the item
 * @param parent Parent modelindex
 * @return Index of the item
 */
QModelIndex MovieModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

/**
 * @brief Clears the current contents
 */
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

/**
 * @brief Returns a list of all movies
 * @return List of movies
 */
QList<Movie*> MovieModel::movies()
{
    return m_movies;
}
