#include "MovieModel.h"

#include <QIcon>
#include <QPainter>
#include "globals/Globals.h"
#include "globals/Helper.h"

/**
 * @brief MovieModel::MovieModel
 * @param parent
 */
MovieModel::MovieModel(QObject *parent) :
    QAbstractItemModel(parent)
{
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
    return 2+MediaStatusLast-MediaStatusFirst;
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

    if (index.column() == 0) {
        if (role == Qt::DisplayRole) {
            return Helper::appendArticle(movie->name());
        } else if (role == Qt::ToolTipRole) {
            if (movie->files().size() == 0)
                return QVariant();
            return movie->files().at(0);
        } else if (role == Qt::UserRole+1) {
            return movie->controller()->infoLoaded();
        } else if (role == Qt::UserRole+2) {
            return movie->hasChanged();
        } else if (role == Qt::UserRole+3) {
            return movie->released();
        } else if (role == Qt::UserRole+4) {
            return movie->watched();
        } else if (role == Qt::UserRole+5) {
            return movie->fileLastModified();
        } else if (role == Qt::UserRole+6) {
            return movie->syncNeeded();
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
    } else if (role == Qt::DecorationRole) {
        QString icon;
        switch (MovieModel::columnToMediaStatus(index.column())) {
        case MediaStatusActors:
            icon = (movie->actors().isEmpty()) ? "actors/red" : "actors/green";
            break;
        case MediaStatusTrailer:
            icon = (movie->trailer().isEmpty()) ? "trailer/red" : "trailer/green";
            break;
        case MediaStatusPoster:
            icon = (movie->hasPoster()) ? "poster/green" : "poster/red";
            break;
        case MediaStatusFanart:
            icon = (movie->hasBackdrop()) ? "fanart/green" : "fanart/red";
            break;
        case MediaStatusExtraArts:
            if (movie->hasCdArt() && movie->hasClearArt() && movie->hasLogo())
                icon = "extraArts/green";
            else if (movie->hasCdArt() || movie->hasClearArt() || movie->hasLogo())
                icon = "extraArts/yellow";
            else
                icon = "extraArts/red";
            break;
        case MediaStatusStreamDetails:
            icon = (movie->streamDetailsLoaded()) ? "streamDetails/green" : "streamDetails/red";
            break;
        case MediaStatusExtraFanarts:
            icon = (movie->hasExtraFanarts()) ? "extraFanarts/green" : "extraFanarts/red";
            break;
        default:
            break;
        }

        if (!icon.isEmpty())
            return QIcon(QPixmap(":mediaStatus/" + icon).scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    } else if (role == Qt::ToolTipRole) {
        return MovieModel::mediaStatusToText(MovieModel::columnToMediaStatus(index.column()));
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

/**
 * @brief Checks if there are new movies (movies where infoLoaded is false)
 * @return True if there are new movies
 */
bool MovieModel::hasNewMovies()
{
    foreach (Movie *movie, m_movies) {
        if (!movie->controller()->infoLoaded())
            return true;
    }

    return false;
}

int MovieModel::mediaStatusToColumn(MediaStatusColumns column)
{
    switch (column) {
    case MediaStatusActors:
        return 7;
        break;
    case MediaStatusExtraArts:
        return 4;
        break;
    case MediaStatusExtraFanarts:
        return 3;
        break;
    case MediaStatusFanart:
        return 2;
        break;
    case MediaStatusPoster:
        return 1;
        break;
    case MediaStatusStreamDetails:
        return 6;
        break;
    case MediaStatusTrailer:
        return 5;
        break;
    default:
        return -1;
        break;
    }
}

MediaStatusColumns MovieModel::columnToMediaStatus(int column)
{
    for (int i=MediaStatusFirst, n=MediaStatusLast ; i<n ; ++i) {
        if (MovieModel::mediaStatusToColumn(static_cast<MediaStatusColumns>(i)) == column)
            return static_cast<MediaStatusColumns>(i);
    }
    return MediaStatusUnknown;
}

QString MovieModel::mediaStatusToText(MediaStatusColumns column)
{
    switch (column) {
    case MediaStatusActors:
        return tr("Actors");
        break;
    case MediaStatusExtraArts:
        return tr("Extra Arts");
        break;
    case MediaStatusExtraFanarts:
        return tr("Extra Fanarts");
        break;
    case MediaStatusFanart:
        return tr("Fanart");
        break;
    case MediaStatusPoster:
        return tr("Poster");
        break;
    case MediaStatusStreamDetails:
        return tr("Stream Details");
        break;
    case MediaStatusTrailer:
        return tr("Trailer");
        break;
    default:
        return QString();
        break;
    }

    return QString();
}
