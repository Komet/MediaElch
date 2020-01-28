#include "MovieModel.h"

#include <QPainter>
#include <numeric>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"

MovieModel::MovieModel(QObject* parent) :
#ifndef Q_OS_WIN
    QAbstractItemModel(parent)
#else
    QAbstractItemModel(parent), m_newIcon(QIcon(":/img/star_blue.png")), m_syncIcon(QIcon(":/img/reload_orange.png"))
#endif
{
#ifndef Q_OS_WIN
    auto font = new MyIconFont(this);
    font->initFontAwesome();
    m_syncIcon = font->icon("refresh_cloud", QColor(248, 148, 6), QColor(255, 255, 255), "", 0, 1.0);
    m_newIcon = font->icon("star", QColor(58, 135, 173), QColor(255, 255, 255), "", 0, 1.0);
#endif
}

/**
 * @brief Adds a movie to the model
 * @param movie Movie to add
 */
void MovieModel::addMovie(Movie* movie)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_movies.append(movie);
    endInsertRows();
    connect(movie, &Movie::sigChanged, this, &MovieModel::onMovieChanged, Qt::UniqueConnection);
}

/**
 * @brief Called when a movies data has changed
 * Emits dataChanged
 * @param movie Movie which has changed
 */
void MovieModel::onMovieChanged(Movie* movie)
{
    const QModelIndex index = createIndex(m_movies.indexOf(movie), 0);
    emit dataChanged(index, index);
}

void MovieModel::update()
{
    const QModelIndex index = createIndex(0, 0);
    emit dataChanged(index, index);
}

/**
 * @brief Get a specific movie
 * @param row Row of the movie
 * @return Movie object
 */
Movie* MovieModel::movie(int row)
{
    if (row < 0 || row >= m_movies.count()) {
        return nullptr;
    }
    return m_movies.at(row);
}

/**
 * @brief Returns the rowcount in our model. (=number of movies)
 * @return Number of rows (=number of movies)
 */
int MovieModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_movies.size();
}

/**
 * @brief Get the column count of our model
 */
int MovieModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    // return roleNames().size();
    return 2 + static_cast<int>(MediaStatusColumn::Last) - static_cast<int>(MediaStatusColumn::First);
}

/**
 * @brief Accesses items data
 * @param index Index of item
 * @param role Role
 * @return data
 */
QVariant MovieModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() > m_movies.count()) {
        return QVariant();
    }
    if (role == Qt::UserRole) {
        return index.row();
    }

    const Movie* const movie = m_movies[index.row()];

    if (index.column() == 0) {
        if (role == Qt::DisplayRole) {
            return helper::appendArticle(movie->name());
        }
        if (role == Qt::ToolTipRole || role == Qt::UserRole + 7) {
            if (movie->files().empty()) {
                return QVariant();
            }
            return movie->files().at(0);
        }
        if (role == Qt::UserRole + 1) {
            return movie->controller()->infoLoaded();
        }
        if (role == Qt::UserRole + 2) {
            return movie->hasChanged();
        }
        if (role == Qt::UserRole + 3) {
            return movie->released();
        }
        if (role == Qt::UserRole + 4) {
            return movie->watched();
        }
        if (role == Qt::UserRole + 5) {
            return movie->fileLastModified();
        }
        if (role == Qt::UserRole + 6) {
            return movie->syncNeeded();
        }
        if (role == Qt::FontRole) {
            if (movie->hasChanged()) {
                QFont font;
                font.setItalic(true);
                return font;
            }
        } else if (role == Qt::DecorationRole) {
            if (!movie->controller()->infoLoaded()) {
                return m_newIcon;
            }
            if (movie->syncNeeded()) {
                return m_syncIcon;
            }
        } else if (role == Qt::BackgroundRole) {
            return helper::colorForLabel(movie->label());
        }
    } else if (role == Qt::DecorationRole) {
        QString icon;

        switch (MovieModel::columnToMediaStatus(index.column())) {
        case MediaStatusColumn::Actors: icon = (movie->actors().isEmpty()) ? "actors/red" : "actors/green"; break;
        case MediaStatusColumn::Trailer: icon = (movie->trailer().isEmpty()) ? "trailer/red" : "trailer/green"; break;
        case MediaStatusColumn::LocalTrailer:
            icon = (movie->hasLocalTrailer()) ? "trailer/green" : "trailer/red";
            break;
        case MediaStatusColumn::Poster:
            icon = (movie->hasImage(ImageType::MoviePoster)) ? "poster/green" : "poster/red";
            break;
        case MediaStatusColumn::Fanart:
            icon = (movie->hasImage(ImageType::MovieBackdrop)) ? "fanart/green" : "fanart/red";
            break;
        case MediaStatusColumn::ExtraArts:
            if (movie->hasImage(ImageType::MovieCdArt) && movie->hasImage(ImageType::MovieClearArt)
                && movie->hasImage(ImageType::MovieLogo) && movie->hasImage(ImageType::MovieBanner)
                && movie->hasImage(ImageType::MovieThumb)) {
                icon = "extraArts/green";
            } else if (movie->hasImage(ImageType::MovieCdArt) || movie->hasImage(ImageType::MovieClearArt)
                       || movie->hasImage(ImageType::MovieLogo) || movie->hasImage(ImageType::MovieBanner)
                       || movie->hasImage(ImageType::MovieThumb)) {
                icon = "extraArts/yellow";
            } else {
                icon = "extraArts/red";
            }
            break;
        case MediaStatusColumn::StreamDetails:
            icon = (movie->streamDetailsLoaded()) ? "streamDetails/green" : "streamDetails/red";
            break;
        case MediaStatusColumn::ExtraFanarts:
            icon = (movie->constImages().hasExtraFanarts()) ? "extraFanarts/green" : "extraFanarts/red";
            break;
        case MediaStatusColumn::Id: icon = movie->hasValidImdbId() ? "id/green" : "id/red"; break;
        default: break;
        }

        if (!icon.isEmpty()) {
            static QHash<QString, QIcon> icons;
            if (!icons.contains(icon)) {
                icons.insert(icon, QIcon(":mediaStatus/" + icon));
            }
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
QModelIndex MovieModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return {};
}

/**
 * @brief Returns a modelindex for the given row and column
 * Parent is not used because our movie model uses only one column.
 * @param row Row of the item
 * @param column Column of the item
 * @param parent Parent modelindex
 * @return Index of the item
 */
QModelIndex MovieModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

/**
 * @brief Clears the current contents
 */
void MovieModel::clear()
{
    if (m_movies.empty()) {
        return;
    }
    beginRemoveRows(QModelIndex(), 0, m_movies.size() - 1);
    for (Movie* movie : m_movies) {
        movie->deleteLater();
    }
    m_movies.clear();
    endRemoveRows();
}

/**
 * @brief Returns a list of all movies
 * @return List of movies
 */
QVector<Movie*> MovieModel::movies()
{
    return m_movies;
}

/// @brief Checks if there are new movies (movies where infoLoaded is false)
/// @return True if there are new movies
int MovieModel::countNewMovies()
{
    const auto checkInfoLoaded = [](const Movie* movie) { return !movie->controller()->infoLoaded(); };
    return static_cast<int>(std::count_if(m_movies.cbegin(), m_movies.cend(), checkInfoLoaded));
}

int MovieModel::mediaStatusToColumn(MediaStatusColumn column)
{
    switch (column) {
    case MediaStatusColumn::Actors: return 9;
    case MediaStatusColumn::ExtraArts: return 5;
    case MediaStatusColumn::ExtraFanarts: return 4;
    case MediaStatusColumn::Fanart: return 3;
    case MediaStatusColumn::Poster: return 2;
    case MediaStatusColumn::StreamDetails: return 8;
    case MediaStatusColumn::Trailer: return 6;
    case MediaStatusColumn::LocalTrailer: return 7;
    case MediaStatusColumn::Id: return 1;
    default: return -1;
    }
}

MediaStatusColumn MovieModel::columnToMediaStatus(int column)
{
    for (int i = static_cast<int>(MediaStatusColumn::First), n = static_cast<int>(MediaStatusColumn::Last); i <= n;
         ++i) {
        if (MovieModel::mediaStatusToColumn(static_cast<MediaStatusColumn>(i)) == column) {
            return static_cast<MediaStatusColumn>(i);
        }
    }
    return MediaStatusColumn::Unknown;
}

QString MovieModel::mediaStatusToText(MediaStatusColumn column)
{
    switch (column) {
    case MediaStatusColumn::Actors: return tr("Actors");
    case MediaStatusColumn::ExtraArts: return tr("Extra Arts");
    case MediaStatusColumn::ExtraFanarts: return tr("Extra Fanarts");
    case MediaStatusColumn::Fanart: return tr("Fanart");
    case MediaStatusColumn::Poster: return tr("Poster");
    case MediaStatusColumn::StreamDetails: return tr("Stream Details");
    case MediaStatusColumn::Trailer: return tr("Trailer");
    case MediaStatusColumn::LocalTrailer: return tr("Local Trailer");
    case MediaStatusColumn::Id: return tr("IMDb ID");
    default: return QString();
    }
}
