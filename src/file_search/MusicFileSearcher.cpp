#include "src/file_search/MusicFileSearcher.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "database/MusicPersistence.h"
#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "log/Log.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QtConcurrent>

MusicFileSearcher::MusicFileSearcher(QObject* parent) :
    QObject(parent), m_progressMessageId{Constants::MusicFileSearcherProgressMessageId}, m_aborted{false}
{
}

void MusicFileSearcher::setMusicDirectories(QVector<mediaelch::MediaDirectory> directories)
{
    const auto& filter = Settings::instance()->advanced()->musicFilters();
    m_directories.clear();
    for (const mediaelch::MediaDirectory& dir : directories) {
        if (filter.isFolderExcluded(dir.path.dirName())) {
            qCWarning(generic) << "[MusicFileSearcher] Music directory is excluded by advanced settings! "
                                  "Is this intended? Directory:"
                               << dir.path.path();

        } else if (!dir.path.isReadable()) {
            qCDebug(generic) << "[MusicFileSearcher] Music directory is not readable, skipping:" << dir.path.path();

        } else {
            qCDebug(generic) << "[MusicFileSearcher] Adding music directory" << dir.path.path();
            m_directories.append(dir);
        }
    }
}

void MusicFileSearcher::reload(bool force)
{
    m_aborted = false;

    emit searchStarted(tr("Searching for Music..."));
    Manager::instance()->musicModel()->clear();

    const auto& filter = Settings::instance()->advanced()->musicFilters();

    QVector<Artist*> artists;
    QVector<Artist*> artistsFromDb;
    QVector<Album*> albums;
    QVector<Album*> albumsFromDb;

    mediaelch::MusicPersistence persistence{*Manager::instance()->database()};

    if (force) {
        persistence.clearAllArtists();
    }

    QMap<Artist*, mediaelch::DirectoryPath> artistPaths;
    QMap<Album*, mediaelch::DirectoryPath> albumPaths;
    for (const mediaelch::MediaDirectory& dir : asConst(m_directories)) {
        if (m_aborted) {
            break;
        }
        if (dir.disabled) {
            continue;
        }

        if (dir.autoReload) {
            persistence.clearArtistsInDirectory(mediaelch::DirectoryPath(dir.path));
        }

        if (dir.autoReload || force) {
            QDirIterator it(dir.path.path(), QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::FollowSymlinks);
            while (it.hasNext()) {
                if (m_aborted) {
                    break;
                }

                it.next();

                if (filter.isFolderExcluded(it.fileInfo().dir().dirName())) {
                    continue;
                }

                emit currentDir(it.fileInfo().baseName());
                auto* artist = new Artist(mediaelch::DirectoryPath(it.filePath()), this);
                artist->setName(it.fileInfo().baseName());
                artists.append(artist);
                artistPaths.insert(artist, mediaelch::DirectoryPath(dir.path));

                QDirIterator itAlbums(it.filePath(), QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::FollowSymlinks);
                while (itAlbums.hasNext()) {
                    itAlbums.next();

                    if (filter.isFolderExcluded(itAlbums.fileInfo().dir().dirName())) {
                        continue;
                    }

                    if (itAlbums.fileInfo().baseName() == "extrafanart") {
                        continue;
                    }
                    if (itAlbums.fileInfo().baseName() == "extrathumbs") {
                        continue;
                    }

                    auto* album = new Album(mediaelch::DirectoryPath(itAlbums.filePath()), this);
                    album->setTitle(itAlbums.fileInfo().baseName());
                    album->setArtistObj(artist);
                    artist->addAlbum(album);
                    albums.append(album);
                    albumPaths.insert(album, mediaelch::DirectoryPath(dir.path));
                }
            }
        } else {
            QVector<Artist*> artistsInPath = persistence.artistsInDirectory(mediaelch::DirectoryPath(dir.path));
            for (Artist* artist : artistsInPath) {
                if (artistsFromDb.count() % 20 == 0) {
                    emit currentDir(artist->path().toString().mid(dir.path.path().length()));
                }
                QVector<Album*> albumsOfArtist = persistence.albums(artist);
                artistsFromDb.append(artist);
                albumsFromDb.append(albumsOfArtist);
            }
        }
    }

    emit currentDir("");
    emit searchStarted(tr("Loading Music..."));

    int current = 0;
    int max = qsizetype_to_int(artists.length() + albums.length() + artistsFromDb.length() + albumsFromDb.length());

    Manager::instance()->database()->db().transaction();
    for (Artist* artist : artists) {
        if (m_aborted) {
            Manager::instance()->database()->db().commit();
            return;
        }
        artist->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
        if (current % 20 == 0) {
            emit currentDir(artist->name());
        }
        emit progress(++current, max, m_progressMessageId);
        persistence.add(artist, artistPaths.value(artist));
    }
    for (Album* album : albums) {
        if (m_aborted) {
            Manager::instance()->database()->db().commit();
            return;
        }
        album->controller()->loadData(Manager::instance()->mediaCenterInterface(), true);
        if (current % 20 == 0) {
            emit currentDir(album->artist() + "/" + album->title());
        }
        emit progress(++current, max, m_progressMessageId);
        persistence.add(album, albumPaths.value(album));
    }
    Manager::instance()->database()->db().commit();

    QtConcurrent::blockingMapped(artistsFromDb, MusicFileSearcher::loadArtistData);
    QtConcurrent::blockingMapped(albumsFromDb, MusicFileSearcher::loadAlbumData);

    artists.append(artistsFromDb);
    albums.append(albumsFromDb);

    QMap<Artist*, MusicModelItem*> artistModelItems;
    for (Artist* artist : artists) {
        MusicModelItem* artistItem = Manager::instance()->musicModel()->appendChild(artist);
        artistModelItems.insert(artist, artistItem);
    }
    for (Album* album : albums) {
        MusicModelItem* artistItem = artistModelItems.value(album->artistObj(), nullptr);
        if (artistItem == nullptr) {
            qCWarning(generic) << "Artist item was not found for album" << album->path();
            continue;
        }
        artistItem->appendChild(album);
    }

    if (!m_aborted) {
        emit musicLoaded();
    }
}

void MusicFileSearcher::abort()
{
    m_aborted = true;
}

Artist* MusicFileSearcher::loadArtistData(Artist* artist)
{
    artist->controller()->loadData(Manager::instance()->mediaCenterInterface(), false, false);
    return artist;
}

Album* MusicFileSearcher::loadAlbumData(Album* album)
{
    album->controller()->loadData(Manager::instance()->mediaCenterInterface(), false, false);
    return album;
}
