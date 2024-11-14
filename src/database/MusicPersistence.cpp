#include "database/MusicPersistence.h"

#include "data/music/Album.h"
#include "data/music/Artist.h"
#include "database/Database.h"
#include "globals/Manager.h"
#include "log/Log.h"

#include <QSqlQuery>
#include <QSqlRecord>

namespace mediaelch {

MusicPersistence::MusicPersistence(Database& db) : m_db(db)
{
}

QSqlDatabase MusicPersistence::db()
{
    return m_db.db();
}

void MusicPersistence::clearAllArtists()
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM artists");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='artists'");
    query.exec();
    clearAllAlbums();
}

void MusicPersistence::clearArtistsInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM artists WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    clearAlbumsInDirectory(path);
}

void MusicPersistence::add(Artist* artist, DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO artists(content, dir, path) "
                  "VALUES(:content, :dir, :path)");
    query.bindValue(":content", artist->nfoContent().isEmpty() ? "" : artist->nfoContent().toUtf8());
    query.bindValue(":dir", artist->path().toString().toUtf8());
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    artist->setDatabaseId(query.lastInsertId().toInt());
}

void MusicPersistence::update(Artist* artist)
{
    QSqlQuery query(db());
    query.prepare("UPDATE artists SET content=:content WHERE idArtist=:id");
    query.bindValue(":content", artist->nfoContent().isEmpty() ? "" : artist->nfoContent());
    query.bindValue(":id", artist->databaseId().toInt());
    query.exec();
}

QVector<Artist*> MusicPersistence::artistsInDirectory(DirectoryPath path)
{
    QVector<Artist*> artists;
    QSqlQuery query(db());
    query.prepare("SELECT idArtist, content, dir FROM artists WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    while (query.next()) {
        mediaelch::DirectoryPath dir(QString::fromUtf8(query.value(query.record().indexOf("dir")).toByteArray()));
        auto* artist = new Artist(dir, Manager::instance()->musicFileSearcher());
        artist->setDatabaseId(query.value(query.record().indexOf("idArtist")).toInt());
        artist->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        artists.append(artist);
    }
    return artists;
}

void MusicPersistence::clearAllAlbums()
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM albums");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='albums'");
    query.exec();
}

void MusicPersistence::clearAlbumsInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM albums WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
}

void MusicPersistence::add(Album* album, DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO albums(idArtist, content, dir, path) "
                  "VALUES(:idArtist, :content, :dir, :path)");
    query.bindValue(":idArtist", album->artistObj()->databaseId().toInt());
    query.bindValue(":content", album->nfoContent().isEmpty() ? "" : album->nfoContent().toUtf8());
    query.bindValue(":dir", album->path().toString().toUtf8());
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    album->setDatabaseId(query.lastInsertId().toInt());
}

void MusicPersistence::update(Album* album)
{
    QSqlQuery query(db());
    query.prepare("UPDATE albums SET content=:content WHERE idAlbum=:id");
    query.bindValue(":content", album->nfoContent().isEmpty() ? "" : album->nfoContent());
    query.bindValue(":id", album->databaseId().toInt());
    query.exec();
}

QVector<Album*> MusicPersistence::albums(Artist* artist)
{
    QVector<Album*> albums;
    QSqlQuery query(db());
    query.prepare("SELECT idAlbum, content, dir FROM albums WHERE idArtist=:idArtist");
    query.bindValue(":idArtist", artist->databaseId().toInt());
    query.exec();
    while (query.next()) {
        mediaelch::DirectoryPath dir(QString::fromUtf8(query.value(query.record().indexOf("dir")).toByteArray()));
        auto* album = new Album(dir, Manager::instance()->musicFileSearcher());
        album->setDatabaseId(query.value(query.record().indexOf("idAlbum")).toInt());
        album->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        album->setArtistObj(artist);
        artist->addAlbum(album);
        albums.append(album);
    }
    return albums;
}


} // namespace mediaelch
