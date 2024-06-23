#include "MovieModel.h"

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "model/MediaStatusColumn.h"
#include "settings/Settings.h"
#include "ui/main/MyIconFont.h"

#include <QPainter>
#include <numeric>

MovieModel::MovieModel(QObject* parent) :
#ifndef Q_OS_WIN
    QAbstractItemModel(parent)
#else
    QAbstractItemModel(parent), m_newIcon(QIcon(":/img/star_blue.png")), m_syncIcon(QIcon(":/img/reload_orange.png"))
#endif
{
#ifndef Q_OS_WIN
    auto* font = new MyIconFont(this);
    font->initFontAwesome();
    m_syncIcon = font->icon("refresh_cloud", QColor(248, 148, 6), QColor(255, 255, 255), "", 0, 1.0);
    m_newIcon = font->icon("star", QColor(58, 135, 173), QColor(255, 255, 255), "", 0, 1.0);
#endif
}

void MovieModel::addMovie(Movie* movie)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_movies.append(movie);
    endInsertRows();
    connect(movie, &Movie::sigChanged, this, &MovieModel::onMovieChanged, Qt::UniqueConnection);
}

void MovieModel::addMovies(const QVector<Movie*>& movies)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + qsizetype_to_int(movies.size()) - 1);
    m_movies.append(movies);
    for (Movie* movie : movies) {
        connect(movie, &Movie::sigChanged, this, &MovieModel::onMovieChanged, Qt::UniqueConnection);
    }
    endInsertRows();
}

/**
 * \brief Called when a movies data has changed
 * Emits dataChanged
 * \param movie Movie which has changed
 */
void MovieModel::onMovieChanged(Movie* movie)
{
    auto movieIndex = qsizetype_to_int(m_movies.indexOf(movie));
    const QModelIndex indexTopLeft = createIndex(movieIndex, 0);
    const QModelIndex indexBottomRight = createIndex(movieIndex, columnCount({}));
    emit dataChanged(indexTopLeft, indexBottomRight);
}

void MovieModel::update()
{
    const QModelIndex index = createIndex(0, 0);
    emit dataChanged(index, index);
}

Movie* MovieModel::movie(int row)
{
    if (row < 0 || row >= m_movies.count()) {
        return nullptr;
    }
    return m_movies.at(row);
}

int MovieModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        // Root has an invalid model index.
        return 0;
    }
    return qsizetype_to_int(m_movies.size());
}

int MovieModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        // Root has an invalid model index.
        return 0;
    }
    // return roleNames().size();
    // Sync Icon + Name + MediaStatusColumns
    return 2 + static_cast<int>(MediaStatusColumn::Last) - static_cast<int>(MediaStatusColumn::First);
}

/**
 * \brief Accesses items data
 * \param index Index of item
 * \param role Role
 * \return data
 */
QVariant MovieModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_movies.count()) {
        return QVariant();
    }
    if (role == Qt::UserRole) {
        return index.row();
    }
    if (!index.isValid()) {
        return QVariant(); // root
    }

    Movie* movie = m_movies[index.row()];

    if (role == Roles::MoviePointerRole) {
        return QVariant::fromValue(movie);
    }

    if (index.column() == 0) {
        if (role == Qt::DisplayRole) {
            return helper::appendArticle(movie->name());
        }
        if (role == Qt::ToolTipRole || role == Roles::FileNameRole) {
            if (movie->files().isEmpty()) {
                return QVariant();
            }
            return movie->files().first().toNativePathString();
        }
        if (role == Roles::InfoLoadedRole) {
            return movie->controller()->infoLoaded();
        }
        if (role == Roles::HasChangedRole) {
            return movie->hasChanged();
        }
        if (role == Roles::ReleasedRole) {
            return movie->released();
        }
        if (role == Roles::HasWatchedRole) {
            return movie->watched();
        }
        if (role == Roles::FileLastModifiedRole) {
            return movie->fileLastModified();
        }
        if (role == Roles::SyncNeededRole) {
            return movie->syncNeeded();
        }
        if (role == Roles::SortTitleRole) {
            // 8: Sort title or the "normalized" title if the former does not exist.
            QString sortTitle = movie->sortTitle();
            if (sortTitle.isEmpty()) {
                return helper::appendArticle(movie->name());
            }
            return sortTitle;
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
            // TODO: Get rid of Settings::instance() here
            return helper::colorForLabel(movie->label(), Settings::instance()->theme());
        }
    } else if (role == Qt::DecorationRole) {
        QString icon;
        MediaStatusState color = MediaStatusState::RED;

        switch (MovieModel::columnToMediaStatus(index.column())) {
        case MediaStatusColumn::Actors:
            color = (!movie->actors().hasActors()) ? MediaStatusState::RED : MediaStatusState::GREEN;
            icon = "edit-image-face-show";
            break;
        case MediaStatusColumn::Trailer:
            color = (movie->trailer().isEmpty()) ? MediaStatusState::RED : MediaStatusState::GREEN;
            icon = "filmgrain";
            break;
        case MediaStatusColumn::LocalTrailer:
            color = (!movie->hasLocalTrailer()) ? MediaStatusState::RED : MediaStatusState::GREEN;
            icon = "filmgrain";
            break;
        case MediaStatusColumn::Subtitles:
            color = movie->hasSubtitles() ? MediaStatusState::GREEN : MediaStatusState::RED;
            icon = "add-subtitle";
            break;
        case MediaStatusColumn::Tags:
            color = (movie->tags().isEmpty()) ? MediaStatusState::RED : MediaStatusState::GREEN;
            icon = "tag";
            break;
        case MediaStatusColumn::Poster:
            color = (!movie->hasImage(ImageType::MoviePoster)) ? MediaStatusState::RED : MediaStatusState::GREEN;
            icon = "viewimage";
            break;
        case MediaStatusColumn::Fanart:
            color = (!movie->hasImage(ImageType::MovieBackdrop)) ? MediaStatusState::RED : MediaStatusState::GREEN;
            icon = "tool_imageeffects";
            break;
        case MediaStatusColumn::ExtraArts:
            if (movie->hasImage(ImageType::MovieCdArt) && movie->hasImage(ImageType::MovieClearArt)
                && movie->hasImage(ImageType::MovieLogo) && movie->hasImage(ImageType::MovieBanner)
                && movie->hasImage(ImageType::MovieThumb)) {
                color = MediaStatusState::GREEN;
            } else if (movie->hasImage(ImageType::MovieCdArt) || movie->hasImage(ImageType::MovieClearArt)
                       || movie->hasImage(ImageType::MovieLogo) || movie->hasImage(ImageType::MovieBanner)
                       || movie->hasImage(ImageType::MovieThumb)) {
                color = MediaStatusState::YELLOW;
            } else {
                color = MediaStatusState::RED;
            }
            icon = "tools-media-optical-burn-image";
            break;
        case MediaStatusColumn::StreamDetails:
            color = (!movie->streamDetailsLoaded()) ? MediaStatusState::RED : MediaStatusState::GREEN;
            icon = "snap-page";
            break;
        case MediaStatusColumn::ExtraFanarts:
            color = (!movie->constImages().hasExtraFanarts()) ? MediaStatusState::RED : MediaStatusState::GREEN;
            icon = "insert-image";
            break;
        case MediaStatusColumn::Id:
            color = (!movie->hasValidImdbId()) ? MediaStatusState::RED : MediaStatusState::GREEN;
            icon = "globe";
            break;
        default: break;
        }

        if (!icon.isEmpty()) {
            static QHash<QPair<QString, MediaStatusState>, QIcon> icons;
            if (!icons.contains({icon, color})) {
                icons.insert({icon, color}, iconWithMediaStatusColor(icon, color));
            }
            return icons.value({icon, color});
        }

    } else if (role == Qt::ToolTipRole) {
        return MovieModel::mediaStatusToText(MovieModel::columnToMediaStatus(index.column()));
    }

    return QVariant();
}

/**
 * \brief Returns an empty modelindex because no item has a parent
 * \param child Childindex
 * \return Modelindex of the parent item
 */
QModelIndex MovieModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child);
    return {};
}

/**
 * \brief Returns a modelindex for the given row and column
 * Parent is not used because our movie model uses only one column.
 * \param row Row of the item
 * \param column Column of the item
 * \param parent Parent modelindex
 * \return Index of the item
 */
QModelIndex MovieModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

void MovieModel::clear()
{
    if (m_movies.isEmpty()) {
        return;
    }
    beginRemoveRows(QModelIndex(), 0, qsizetype_to_int(m_movies.size()) - 1);
    for (Movie* movie : asConst(m_movies)) {
        movie->deleteLater();
    }
    m_movies.clear();
    endRemoveRows();
}

QVector<Movie*> MovieModel::movies()
{
    return m_movies;
}

/// \brief Checks if there are new movies (movies where infoLoaded is false)
/// \return True if there are new movies
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
    case MediaStatusColumn::Subtitles: return 10;
    case MediaStatusColumn::Tags: return 11;
    case MediaStatusColumn::Unknown: return -1;
    }
    return -1;
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
    case MediaStatusColumn::Subtitles: return tr("Subtitles");
    case MediaStatusColumn::Tags: return tr("Tags");
    case MediaStatusColumn::Id: return tr("IMDb ID");
    case MediaStatusColumn::Unknown: return {};
    }
    return {};
}
