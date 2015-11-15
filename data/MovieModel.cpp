#include "MovieModel.h"

#include <QPainter>
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"

/**
 * @brief MovieModel::MovieModel
 * @param parent
 */
MovieModel::MovieModel(QObject *parent) :
    QAbstractItemModel(parent)
{
#ifdef Q_OS_WIN
    m_newIcon = QIcon(":/img/star_blue.png");
    m_syncIcon = QIcon(":/img/reload_orange.png");
#else
    MyIconFont *font = new MyIconFont(this);
    font->initFontAwesome();
    m_syncIcon = font->icon("refresh_cloud", QColor(248, 148, 6), QColor(255, 255, 255), "", 0, 1.0);
    m_newIcon = font->icon("star", QColor(58, 135, 173), QColor(255, 255, 255), "", 0, 1.0);
#endif
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

void MovieModel::update()
{
    QModelIndex index = createIndex(0, 0);
    emit dataChanged(index, index);
}

/**
 * @brief Get a specific movie
 * @param row Row of the movie
 * @return Movie object
 */
Movie *MovieModel::movie(int row)
{
    if (row < 0 || row >= m_movies.count())
        return 0;
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
            return Helper::instance()->appendArticle(movie->name());
        } else if (role == Qt::ToolTipRole || role == Qt::UserRole+7) {
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
        } else if (role == Qt::FontRole) {
            if (movie->hasChanged()) {
                QFont font;
                font.setItalic(true);
                return font;
            }
        } else if (role == Qt::DecorationRole) {
            if (!movie->controller()->infoLoaded())
                return m_newIcon;
            else if (movie->syncNeeded())
                return m_syncIcon;
        } else if (role == Qt::BackgroundRole) {
            return Helper::instance()->colorForLabel(movie->label());
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
        case MediaStatusLocalTrailer:
            icon = (movie->hasLocalTrailer()) ? "trailer/green" : "trailer/red";
            break;
        case MediaStatusPoster:
            icon = (movie->hasImage(ImageType::MoviePoster)) ? "poster/green" : "poster/red";
            break;
        case MediaStatusFanart:
            icon = (movie->hasImage(ImageType::MovieBackdrop)) ? "fanart/green" : "fanart/red";
            break;
        case MediaStatusExtraArts:
            if (movie->hasImage(ImageType::MovieCdArt) && movie->hasImage(ImageType::MovieClearArt) && movie->hasImage(ImageType::MovieLogo) &&
                    movie->hasImage(ImageType::MovieBanner) && movie->hasImage(ImageType::MovieThumb))
                icon = "extraArts/green";
            else if (movie->hasImage(ImageType::MovieCdArt) || movie->hasImage(ImageType::MovieClearArt) || movie->hasImage(ImageType::MovieLogo) ||
                     movie->hasImage(ImageType::MovieBanner) || movie->hasImage(ImageType::MovieThumb))
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
        case MediaStatusId:
            icon = (movie->id().isEmpty()) ? "id/red" : "id/green";
            break;
        default:
            break;
        }

        if (!icon.isEmpty()) {
            static QHash<QString, QIcon> icons;
            if (!icons.contains(icon))
                icons.insert(icon, QIcon(":mediaStatus/" + icon));
            return icons.value(icon);
        }

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
        movie->deleteLater();
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
int MovieModel::hasNewMovies()
{
    int newMovies = 0;
    foreach (Movie *movie, m_movies) {
        if (!movie->controller()->infoLoaded())
            newMovies++;
    }

    return newMovies;
}

int MovieModel::mediaStatusToColumn(MediaStatusColumns column)
{
    switch (column) {
    case MediaStatusActors:
        return 9;
        break;
    case MediaStatusExtraArts:
        return 5;
        break;
    case MediaStatusExtraFanarts:
        return 4;
        break;
    case MediaStatusFanart:
        return 3;
        break;
    case MediaStatusPoster:
        return 2;
        break;
    case MediaStatusStreamDetails:
        return 8;
        break;
    case MediaStatusTrailer:
        return 6;
        break;
    case MediaStatusLocalTrailer:
        return 7;
        break;
    case MediaStatusId:
        return 1;
        break;
    default:
        return -1;
        break;
    }
}

MediaStatusColumns MovieModel::columnToMediaStatus(int column)
{
    for (int i=MediaStatusFirst, n=MediaStatusLast ; i<=n ; ++i) {
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
    case MediaStatusExtraArts:
        return tr("Extra Arts");
    case MediaStatusExtraFanarts:
        return tr("Extra Fanarts");
    case MediaStatusFanart:
        return tr("Fanart");
    case MediaStatusPoster:
        return tr("Poster");
    case MediaStatusStreamDetails:
        return tr("Stream Details");
    case MediaStatusTrailer:
        return tr("Trailer");
    case MediaStatusLocalTrailer:
        return tr("Local Trailer");
    case MediaStatusId:
        return tr("IMDB ID");
    default:
        return QString();
    }
}
