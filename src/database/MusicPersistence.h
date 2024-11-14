#pragma once

#include "media/Path.h"

#include <QObject>
#include <QSqlDatabase>

class Artist;
class Album;
class Database;

namespace mediaelch {

class MusicPersistence
{
public:
    explicit MusicPersistence(Database& db);
    QSqlDatabase db();

    void clearAllArtists();
    void clearArtistsInDirectory(DirectoryPath path);
    void add(Artist* artist, DirectoryPath path);
    void update(Artist* artist);
    QVector<Artist*> artistsInDirectory(DirectoryPath path);

    void clearAllAlbums();
    void clearAlbumsInDirectory(DirectoryPath path);
    void add(Album* album, DirectoryPath path);
    void update(Album* album);
    QVector<Album*> albums(Artist* artist);

private:
    Database& m_db;
};

} // namespace mediaelch
