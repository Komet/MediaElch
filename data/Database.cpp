#include "Database.h"

#include <QDesktopServices>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include "globals/Manager.h"

/**
 * @brief Database::Database
 * @param parent
 */
Database::Database(QObject *parent) :
    QObject(parent)
{
    QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    QDir dir(dataLocation);
    if (!dir.exists())
        dir.mkpath(dataLocation);
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "mediaDb"));
    m_db->setDatabaseName(dataLocation + QDir::separator() + "MediaElch.sqlite");
    if (!m_db->open()) {
        qWarning() << "Could not open cache database";
    } else {
        QSqlQuery query(*m_db);

        query.prepare("DROP TABLE IF EXISTS movieFiles;");
        query.exec();
        query.prepare("DROP TABLE IF EXISTS movieDirs;");
        query.exec();
        query.prepare("DROP TABLE IF EXISTS tvShowFiles;");
        query.exec();
        query.prepare("DROP TABLE IF EXISTS tvShowDirs;");
        query.exec();
        query.prepare("DROP TABLE IF EXISTS concertFiles;");
        query.exec();
        query.prepare("DROP TABLE IF EXISTS concertDirs;");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS movies ( "
                      "\"idMovie\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"files\" text NOT NULL, "
                      "\"content\" text NOT NULL, "
                      "\"lastModified\" integer NOT NULL, "
                      "\"inSeparateFolder\" integer NOT NULL, "
                      "\"hasPoster\" integer NOT NULL, "
                      "\"hasBackdrop\" integer NOT NULL, "
                      "\"hasLogo\" integer NOT NULL, "
                      "\"hasClearArt\" integer NOT NULL, "
                      "\"hasCdArt\" integer NOT NULL, "
                      "\"path\" text NOT NULL);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS concerts ( "
                      "\"idConcert\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"files\" text NOT NULL, "
                      "\"content\" text NOT NULL, "
                      "\"inSeparateFolder\" integer NOT NULL, "
                      "\"path\" text NOT NULL);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS shows ( "
                      "\"idShow\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"dir\" text NOT NULL, "
                      "\"content\" text NOT NULL, "
                      "\"path\" text NOT NULL);");
        query.exec();

        query.prepare("CREATE TABLE IF NOT EXISTS episodes ( "
                      "\"idEpisode\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"files\" text NOT NULL, "
                      "\"content\" text NOT NULL, "
                      "\"idShow\" integer NOT NULL, "
                      "\"path\" text NOT NULL);");
        query.exec();
    }
}

/**
 * @brief Database::~Database
 */
Database::~Database()
{
    if (m_db && m_db->isOpen()) {
        m_db->close();
        delete m_db;
        m_db = 0;
    }
}

/**
 * @brief Returns an object to the cache database
 * @return Cache database object
 */
QSqlDatabase Database::db()
{
    return *m_db;
}

void Database::clearMovies(QString path)
{
    QSqlQuery query(db());
    if (!path.isEmpty()) {
        query.prepare("DELETE FROM movies WHERE path=:path");
        query.bindValue(":path", path);
        query.exec();
    } else {
        query.prepare("DELETE FROM movies");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='movies'");
        query.exec();
    }
}

void Database::add(Movie *movie, QString path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO movies(files, content, lastModified, inSeparateFolder, hasPoster, hasBackdrop, hasLogo, hasClearArt, hasCdArt, path) "
                  "VALUES(:files, :content, :lastModified, :inSeparateFolder, :hasPoster, :hasBackdrop, :hasLogo, :hasClearArt, :hasCdArt, :path)");
    query.bindValue(":files", movie->files().join(","));
    query.bindValue(":content", movie->nfoContent().isEmpty() ? "" : movie->nfoContent());
    query.bindValue(":lastModified", movie->fileLastModified());
    query.bindValue(":inSeparateFolder", (movie->inSeparateFolder() ? 1 : 0));
    query.bindValue(":hasPoster", movie->hasPoster() ? 1 : 0);
    query.bindValue(":hasBackdrop", movie->hasBackdrop() ? 1 : 0);
    query.bindValue(":hasLogo", movie->hasLogo() ? 1 : 0);
    query.bindValue(":hasClearArt", movie->hasClearArt() ? 1 : 0);
    query.bindValue(":hasCdArt", movie->hasCdArt() ? 1 : 0);
    query.bindValue(":path", path);
    query.exec();
    movie->setDatabaseId(query.lastInsertId().toInt());
}

void Database::update(Movie *movie)
{
    QSqlQuery query(db());
    query.prepare("UPDATE movies SET content=:content WHERE idMovie=:id");
    query.bindValue(":content", movie->nfoContent().isEmpty() ? "" : movie->nfoContent().toLatin1());
    query.bindValue(":idMovie", movie->databaseId());
    query.exec();
}

QList<Movie*> Database::movies(QString path)
{
    QList<Movie*> movies;
    QSqlQuery query(db());
    query.prepare("SELECT idMovie, files, content, lastModified, inSeparateFolder, hasPoster, hasBackdrop, hasLogo, hasClearArt, hasCdArt FROM movies WHERE path=:path");
    query.bindValue(":path", path);
    query.exec();
    while (query.next()) {
        Movie *movie = new Movie(query.value(query.record().indexOf("files")).toString().split(","), Manager::instance()->movieFileSearcher());
        movie->setDatabaseId(query.value(query.record().indexOf("idMovie")).toInt());
        movie->setFileLastModified(query.value(query.record().indexOf("lastModified")).toDateTime());
        movie->setInSeparateFolder(query.value(query.record().indexOf("inSeparateFolder")).toInt() == 1);
        movie->setNfoContent(query.value(query.record().indexOf("content")).toString());
        movie->setHasPoster(query.value(query.record().indexOf("hasPoster")).toInt() == 1);
        movie->setHasBackdrop(query.value(query.record().indexOf("hasBackdrop")).toInt() == 1);
        movie->setHasLogo(query.value(query.record().indexOf("hasLogo")).toInt() == 1);
        movie->setHasClearArt(query.value(query.record().indexOf("hasClearArt")).toInt() == 1);
        movie->setHasCdArt(query.value(query.record().indexOf("hasCdArt")).toInt() == 1);
        movies.append(movie);
    }
    return movies;
}

void Database::clearConcerts(QString path)
{
    QSqlQuery query(db());
    if (!path.isEmpty()) {
        query.prepare("DELETE FROM concerts WHERE path=:path");
        query.bindValue(":path", path);
        query.exec();
    } else {
        query.prepare("DELETE FROM concerts");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='concerts'");
        query.exec();
    }
}

void Database::add(Concert *concert, QString path)
{
    qDebug() << "adding concert" << concert->files();
    QSqlQuery query(db());
    query.prepare("INSERT INTO concerts(files, content, inSeparateFolder, path) "
                  "VALUES(:files, :content, :inSeparateFolder, :path)");
    query.bindValue(":files", concert->files().join(","));
    query.bindValue(":content", concert->nfoContent().isEmpty() ? "" : concert->nfoContent());
    query.bindValue(":inSeparateFolder", (concert->inSeparateFolder() ? 1 : 0));
    query.bindValue(":path", path);
    query.exec();
    qDebug() << query.lastError();
    concert->setDatabaseId(query.lastInsertId().toInt());
}

void Database::update(Concert *concert)
{
    QSqlQuery query(db());
    query.prepare("UPDATE concerts SET content=:content WHERE idConcert=:id");
    query.bindValue(":content", concert->nfoContent().isEmpty() ? "" : concert->nfoContent().toLatin1());
    query.bindValue(":id", concert->databaseId());
    query.exec();
}

QList<Concert*> Database::concerts(QString path)
{
    QList<Concert*> concerts;
    QSqlQuery query(db());
    query.prepare("SELECT idConcert, files, content, inSeparateFolder FROM concerts WHERE path=:path");
    query.bindValue(":path", path);
    query.exec();
    while (query.next()) {
        Concert *concert = new Concert(query.value(query.record().indexOf("files")).toString().split(","), Manager::instance()->concertFileSearcher());
        concert->setDatabaseId(query.value(query.record().indexOf("idConcert")).toInt());
        concert->setInSeparateFolder(query.value(query.record().indexOf("inSeparateFolder")).toInt() == 1);
        concert->setNfoContent(query.value(query.record().indexOf("content")).toString());
        concerts.append(concert);
    }
    return concerts;
}

void Database::add(TvShow *show, QString path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO shows(dir, content, path) "
                  "VALUES(:dir, :content, :path)");
    query.bindValue(":dir", show->dir());
    query.bindValue(":content", show->nfoContent().isEmpty() ? "" : show->nfoContent());
    query.bindValue(":path", path);
    query.exec();
    show->setDatabaseId(query.lastInsertId().toInt());
}

void Database::add(TvShowEpisode *episode, QString path, int idShow)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO episodes(files, content, idShow, path) "
                  "VALUES(:files, :content, :idShow, :path)");
    query.bindValue(":files", episode->files().join(","));
    query.bindValue(":content", episode->nfoContent().isEmpty() ? "" : episode->nfoContent());
    query.bindValue(":idShow", idShow);
    query.bindValue(":path", path);
    query.exec();
    episode->setDatabaseId(query.lastInsertId().toInt());
}

void Database::update(TvShow *show)
{
    QSqlQuery query(db());
    query.prepare("UPDATE shows SET content=:content WHERE idShow=:id");
    query.bindValue(":content", show->nfoContent().isEmpty() ? "" : show->nfoContent().toLatin1());
    query.bindValue(":id", show->databaseId());
    query.exec();
}

void Database::update(TvShowEpisode *episode)
{
    QSqlQuery query(db());
    query.prepare("UPDATE episodes SET content=:content WHERE idEpisode=:id");
    query.bindValue(":content", episode->nfoContent().isEmpty() ? "" : episode->nfoContent().toLatin1());
    query.bindValue(":id", episode->databaseId());
    query.exec();
}

QList<TvShow*> Database::shows(QString path)
{
    QList<TvShow*> shows;
    QSqlQuery query(db());
    query.prepare("SELECT idShow, dir, content, path FROM shows WHERE path=:path");
    query.bindValue(":path", path);
    query.exec();
    while (query.next()) {
        TvShow *show = new TvShow(query.value(query.record().indexOf("dir")).toString(), Manager::instance()->tvShowFileSearcher());
        show->setDatabaseId(query.value(query.record().indexOf("idShow")).toInt());
        show->setNfoContent(query.value(query.record().indexOf("content")).toString());
        shows.append(show);
    }
    return shows;
}

QList<TvShowEpisode*> Database::episodes(int idShow)
{
    QList<TvShowEpisode*> episodes;
    QSqlQuery query(db());
    query.prepare("SELECT idEpisode, files, content FROM episodes WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();
    while (query.next()) {
        TvShowEpisode *episode = new TvShowEpisode(query.value(query.record().indexOf("files")).toString().split(","));
        episode->setDatabaseId(query.value(query.record().indexOf("idEpisode")).toInt());
        episode->setNfoContent(query.value(query.record().indexOf("content")).toString());
        episodes.append(episode);
    }
    return episodes;
}

void Database::clearTvShows(QString path)
{
    QSqlQuery query(db());
    if (!path.isEmpty()) {
        query.prepare("DELETE FROM shows WHERE path=:path");
        query.bindValue(":path", path);
        query.exec();
        query.prepare("DELETE FROM episodes WHERE path=:path");
        query.bindValue(":path", path);
        query.exec();
    } else {
        query.prepare("DELETE FROM shows");
        query.exec();
        query.prepare("DELETE FROM episodes");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='shows'");
        query.exec();
        query.prepare("DELETE FROM sqlite_sequence WHERE name='episodes'");
        query.exec();
    }
}

int Database::episodeCount()
{
    QSqlQuery query(db());
    query.prepare("SELECT COUNT(*) FROM episodes");
    query.exec();
    query.next();
    return query.value(0).toInt();
}
