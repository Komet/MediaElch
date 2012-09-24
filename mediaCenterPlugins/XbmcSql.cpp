#include <QDir>
#include <QDomDocument>
#include <QFileInfo>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QXmlStreamWriter>
#include <QtSql>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "main/MessageBox.h"
#include "settings/Settings.h"
#include "XbmcSql.h"

/**
 * @brief XbmcSql::XbmcSql
 * @param parent
 * @param connectionName
 */
XbmcSql::XbmcSql(QObject *parent, QString connectionName)
{
    setParent(parent);
    m_db = 0;
    m_isMySQL = false;
    m_connectionName = connectionName;
}

/**
 * @brief XbmcSql::~XbmcSql
 */
XbmcSql::~XbmcSql()
{
}

/**
 * @brief Checks if our MediaCenterPlugin supports a feature
 * @param feature Feature to check
 * @return Feature is supported or not
 */
bool XbmcSql::hasFeature(int feature)
{
    if (feature == MediaCenterFeatures::EditTvShowEpisodeCertification)
        return false;
    if (feature == MediaCenterFeatures::EditTvShowEpisodeShowTitle)
        return false;
    if (feature == MediaCenterFeatures::EditTvShowEpisodeNetwork)
        return false;

    if (feature == MediaCenterFeatures::EditConcertRating)
        return false;
    if (feature == MediaCenterFeatures::EditConcertTagline)
        return false;
    if (feature == MediaCenterFeatures::EditConcertCertification)
        return false;
    if (feature == MediaCenterFeatures::EditConcertTrailer)
        return false;
    if (feature == MediaCenterFeatures::EditConcertWatched)
        return false;

    return true;
}

/**
 * @brief Connects to a MySQL database
 * @param host Hostname
 * @param database Database name
 * @param username Username
 * @param password Password
 */
void XbmcSql::connectMysql(QString host, QString database, QString username, QString password)
{
    qDebug() << "Entered, host=" << host << "database=" << database << "username=" << username;
    if (m_db) {
        qDebug() << "DB already connected, closing";
        m_db->close();
        delete m_db;
    }

    m_isMySQL = true;
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL", m_connectionName));
    m_db->setHostName(host);
    m_db->setDatabaseName(database);
    m_db->setUserName(username);
    m_db->setPassword(password);
    if (!m_db->open())
        MessageBox::instance()->showMessage(tr("Connection to XBMC MySQL Database failed! \"%1\"").arg(m_db->lastError().text()));
}

/**
 * @brief Connects to a sqlite database
 * @param database Path to the database
 */
void XbmcSql::connectSqlite(QString database)
{
    qDebug() << "Entered, database=" << database;
    if (m_db) {
        qDebug() << "DB already connected, closing";
        m_db->close();
        delete m_db;
    }
    m_isMySQL = false;
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", m_connectionName));
    m_db->setDatabaseName(database);
    if (!m_db->open())
        MessageBox::instance()->showMessage(tr("Connection to XBMC SQLite Database failed! \"%1\"").arg(m_db->lastError().text()));
}

/**
 * @brief Called when MediaElch shutsdown
 * (There were problems shutting down the db connection in the destructor)
 */
void XbmcSql::shutdown()
{
    qDebug() << "Entered";
    if (m_db) {
        m_db->close();
        QString connection = m_db->connectionName();
        delete m_db;
        QSqlDatabase::removeDatabase(connection);
    }
}

/**
 * @brief Saves movie information
 * @param movie The movie to save
 * @return Saving success
 */
bool XbmcSql::saveMovie(Movie *movie)
{
    qDebug() << "Entered, movie=" << movie->name();
    QSqlQuery query(db());

    // get Path ID
    int idPath = -1;
    int mediaCenterId = movie->mediaCenterId();
    qDebug() << "mediaCenterId=" << mediaCenterId;
    if (mediaCenterId != -1) {
        query.prepare("SELECT c23 FROM movie WHERE idMovie=:idMovie");
        query.bindValue(":idMovie", movie->mediaCenterId());
        query.exec();
        if (query.next()) {
            idPath = query.value(0).toInt();
            qDebug() << "Got path id based on idMovie, idPath=" << idPath;
        }
    }
    QFileInfo fiPath(mediaCenterPath(movie->files().at(0)));
    qDebug() << "fiPath=" << fiPath.absoluteFilePath();
    if (idPath == -1) {
        qDebug() << "idPath is -1";
        QString path = fiPath.path();
        qDebug() << "path=" << path;
        QString mediaCenterPath2 = mediaCenterPath(movie->files().at(0));
        qDebug() << "mediaCenterPath=" << mediaCenterPath2;
        if (mediaCenterPath2.contains("\\")) {
            qDebug() << "mediaCenterPath contains \\";
            path.replace("/", "\\");
            if (!path.endsWith("\\"))
                path.append("\\");
        } else {
            path.replace("\\", "/");
            if (!path.endsWith("/"))
                path.append("/");
        }
        qDebug() << "path is now" << path;

        query.prepare("SELECT idPath FROM path WHERE strPath=:path");
        query.bindValue(":path", path);
        query.exec();
        qDebug() << query.lastQuery() << ":path=" << path;
        if (query.next()) {
            idPath = query.value(query.record().indexOf("idPath")).toInt();
            qDebug() << "Got path, idPath=" << idPath;
        } else {
            query.prepare("INSERT INTO path(strPath, strContent, strScraper, scanRecursive, useFolderNames, noUpdate, exclude) "
                          "VALUES(:path, 'movies', 'metadata.themoviedb.org', 0, 0, 1, 0)");
            query.bindValue(":path", path);
            query.exec();
            idPath = query.lastInsertId().toInt();
            qDebug() << "Inserted path" << path << "got idPath=" << idPath;
        }
    }

    // get File ID
    int idMovie = -1;
    int idFile = -1;
    if (movie->mediaCenterId() != -1) {
        idMovie = movie->mediaCenterId();
        query.prepare("SELECT idFile FROM movie WHERE idMovie=:idMovie");
        query.bindValue(":idMovie", idMovie);
        query.exec();
        qDebug() << query.lastQuery() << ":idMovie=" << idMovie;
        if (query.next()) {
            idFile = query.value(0).toInt();
            qDebug() << "Got idFile=" << idFile;
        }
    } else {
        if (movie->files().count() == 1) {
            qDebug() << "Movie is single file";
            query.prepare("SELECT idFile, strFilename FROM files WHERE strFilename=:fileName AND idPath=:idPath");
            query.bindValue(":idPath", idPath);
            query.bindValue(":fileName", fiPath.fileName());
            query.exec();
            qDebug() << query.lastQuery() << ":fileName=" << fiPath.fileName() << ":idPath=" << idPath;
        } else {
            qDebug() << "Movie contains multiple files";
            query.prepare(QString("SELECT idFile, strFilename FROM files WHERE strFilename LIKE 'stack://%%1%' AND idPath='%2'").arg(fiPath.fileName().replace("'", "''")).arg(idPath));
            query.exec();
            qDebug() << query.lastQuery();
        }
        if (movie->files().count() == 1) {
            if (query.next()) {
                idFile = query.value(query.record().indexOf("idFile")).toInt();
                qDebug() << "Got (single file) idFile=" << idFile;
            }
        } else {
            while (idFile == -1 && query.next()) {
                qDebug() << "Checking if entry matches all files";
                QString path = query.value(query.record().indexOf("strFilename")).toString();
                path.replace("stack://", "");
                qDebug() << "Path is now" << path;
                QStringList dbPaths = path.split(" , ", QString::SkipEmptyParts);
                QStringList filePaths;
                foreach (const QString &path, movie->files())
                    filePaths << mediaCenterPath(path);
                qSort(filePaths);
                qSort(dbPaths);
                qDebug() << "filePaths=" << filePaths;
                qDebug() << "dbPaths=" << dbPaths;
                if (dbPaths == filePaths) {
                    idFile = query.value(query.record().indexOf("idFile")).toInt();
                    qDebug() << "FilePaths and dbPaths match, idFile=" << idFile;
                }
            }
        }
    }

    // update file data and get movie id, or insert file
    if (idFile != -1) {
        qDebug() << "We have idFile=" << idFile;
        query.prepare("SELECT idMovie FROM movie WHERE idFile=:idFile");
        query.bindValue(":idFile", idFile);
        query.exec();
        if (query.next()) {
            idMovie = query.value(query.record().indexOf("idMovie")).toInt();
            qDebug() << "Got idMovie=" << idMovie;
        }
        query.prepare("UPDATE files SET playCount=:playCount, lastPlayed=:lastPlayed WHERE idFile=:idFile");
        query.bindValue(":idFile", idFile);
        if (movie->playcount() == 0)
            query.bindValue(":playCount", QVariant(QVariant::Int));
        else
            query.bindValue(":playCount", movie->playcount());
        query.bindValue(":lastPlayed", movie->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
        query.exec();
    } else {
        qDebug() << "Inserting new file";
        QString filename;
        if (movie->files().count() == 1) {
            QFileInfo fiFile(movie->files().at(0));
            filename = fiFile.fileName();
        } else {
            QStringList files;
            foreach (const QString &file, movie->files())
                files << mediaCenterPath(file);
            filename = QString("stack://%1").arg(files.join(" , "));
        }
        query.prepare("INSERT INTO files(idPath, strFilename, playCount, lastPlayed) VALUES(:idPath, :filename, :playCount, :lastPlayed)");
        query.bindValue(":idPath", idPath);
        query.bindValue(":filename", filename);
        if (movie->playcount() == 0)
            query.bindValue(":playCount", QVariant(QVariant::Int));
        else
            query.bindValue(":playCount", movie->playcount());
        query.bindValue(":lastPlayed", movie->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
        query.exec();
        qDebug() << query.lastQuery() << ":idPath=" << idPath << ":filename=" << filename;
        idFile = query.lastInsertId().toInt();
        qDebug() << "idFile=" << idFile;
    }

    // create xml data for thumbnails and fanart
    QByteArray thumbnails;
    QByteArray fanart;
    QXmlStreamWriter xml(&thumbnails);
    QXmlStreamWriter xml2(&fanart);

    foreach (const Poster &poster, movie->posters()) {
        xml.writeStartElement("thumb");
        xml.writeAttribute("preview", poster.thumbUrl.toString());
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
    }
    xml2.writeStartElement("fanart");
    foreach (const Poster &poster, movie->backdrops()) {
        xml2.writeStartElement("thumb");
        xml2.writeAttribute("preview", poster.thumbUrl.toString());
        xml2.writeCharacters(poster.originalUrl.toString());
        xml2.writeEndElement();
    }
    xml2.writeEndElement();

    // update movie info or insert new movie
    if (idMovie != -1) {
        qDebug() << "We have idMovie, updating...";
        query.prepare("UPDATE movie SET "
                      "c00=:title, "
                      "c01=:plot, "
                      "c03=:tagline, "
                      "c05=:rating, "
                      "c07=:year, "
                      "c08=:thumbnails, "
                      "c10=:sortTitle, "
                      "c11=:runtime, "
                      "c12=:certification, "
                      "c14=:genres, "
                      "c16=:originalTitle, "
                      "c18=:studios, "
                      "c19=:trailer, "
                      "c20=:fanart, "
                      "c21=:countries "
                      "WHERE idMovie=:movieId ");
        query.bindValue(":movieId", idMovie);
        query.bindValue(":title", movie->name());
        query.bindValue(":plot", movie->overview());
        query.bindValue(":tagline", movie->tagline());
        query.bindValue(":rating", movie->rating());
        query.bindValue(":year", movie->released().toString("yyyy"));
        query.bindValue(":thumbnails", QString(thumbnails));
        if (movie->sortTitle().isEmpty())
            query.bindValue(":sortTitle", QVariant(QVariant::String));
        else
            query.bindValue(":sortTitle", movie->sortTitle());
        if (movie->runtime() == 0)
            query.bindValue(":runtime", QVariant(QVariant::Int));
        else
            query.bindValue(":runtime", movie->runtime());
        query.bindValue(":certification", movie->certification());
        query.bindValue(":genres", movie->genres().join(" / "));
        query.bindValue(":originalTitle", movie->originalName());
        query.bindValue(":studios", movie->studios().join(" / "));
        query.bindValue(":trailer", Helper::formatTrailerUrl(movie->trailer().toString()));
        query.bindValue(":fanart", QString(fanart));
        query.bindValue(":countries", movie->countries().join(" / "));
        query.exec();
    } else {
        qDebug() << "No idMovie, inserting a new movie";
        QString file;
        if (movie->files().count() == 1) {
            qDebug() << "It's a single file" << movie->files().at(0);
            file = mediaCenterPath(movie->files().at(0));
            qDebug() << "mediaCenterPath is " << file;
            bool isUnixFile = !file.contains("\\");
            qDebug() << "isUnixFile=" << isUnixFile;
            QStringList fileSplit = (isUnixFile) ? file.split("/", QString::SkipEmptyParts) : file.split("\\", QString::SkipEmptyParts);
            qDebug() << "fileSplit=" << fileSplit;
            if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0 ||
                                         QString::compare(fileSplit.last(), "index.bdmv", Qt::CaseInsensitive) == 0)) {
                fileSplit.takeLast();
                if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0 ||
                                             QString::compare(fileSplit.last(), "BDMV", Qt::CaseInsensitive) == 0)) {
                    fileSplit.takeLast();
                }
                file = (isUnixFile) ? fileSplit.join("/") : fileSplit.join("\\");
                file.prepend((isUnixFile) ? "/" : "\\");
                file.append((isUnixFile) ? "/" : "\\");
            }
        } else {
            QStringList files;
            foreach (const QString &mFile, movie->files()) {
                qDebug() << mFile << "becomes" << mediaCenterPath(mFile);
                files << mediaCenterPath(mFile);
            }
            file = "stack://" + files.join(" , ");
        }
        qDebug() << "file=" << file;

        query.prepare("INSERT INTO movie(idFile, c00, c01, c03, c05, c07, c08, c10, c11, c12, c14, c16, c18, c19, c20, c21, c22, c23) "
                      "VALUES(:idFile, :title, :plot, :tagline, :rating, :year, :thumbnails, :sortTitle, :runtime, :certification, :genres, :originalTitle, "
                      ":studios, :trailer, :fanart, :countries, :file, :idPath)");
        query.bindValue(":idFile", idFile);
        query.bindValue(":title", movie->name());
        query.bindValue(":plot", movie->overview());
        query.bindValue(":tagline", movie->tagline());
        query.bindValue(":rating", movie->rating());
        query.bindValue(":year", movie->released().toString("yyyy"));
        query.bindValue(":thumbnails", QString(thumbnails));
        if (movie->sortTitle().isEmpty())
            query.bindValue(":sortTitle", QVariant(QVariant::String));
        else
            query.bindValue(":sortTitle", movie->sortTitle());
        if (movie->runtime() == 0)
            query.bindValue(":runtime", QVariant(QVariant::Int));
        else
            query.bindValue(":runtime", movie->runtime());
        query.bindValue(":certification", movie->certification());
        query.bindValue(":genres", movie->genres().join(" / "));
        query.bindValue(":originalTitle", movie->originalName());
        query.bindValue(":studios", movie->studios().join(" / "));
        query.bindValue(":trailer", Helper::formatTrailerUrl(movie->trailer().toString()));
        query.bindValue(":fanart", QString(fanart));
        query.bindValue(":countries", movie->countries().join(" / "));
        query.bindValue(":file", file);
        query.bindValue(":idPath", idPath);
        query.exec();
        idMovie = query.lastInsertId().toInt();
        qDebug() << "idMovie=" << idMovie;
    }

    // Studios
    query.prepare("DELETE FROM studiolinkmovie WHERE idMovie=:idMovie");
    query.bindValue(":idMovie", idMovie);
    query.exec();
    foreach (const QString &studio, movie->studios()) {
        int idStudio = -1;
        query.prepare("SELECT idStudio FROM studio WHERE strStudio=:studio");
        query.bindValue(":studio", studio);
        query.exec();
        if (query.next()) {
            idStudio = query.value(query.record().indexOf("idStudio")).toInt();
        } else {
            query.prepare("INSERT INTO studio(strStudio) VALUES(:studio)");
            query.bindValue(":studio", studio);
            query.exec();
            idStudio = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO studiolinkmovie(idStudio, idMovie) VALUES(:idStudio, :idMovie)");
        query.bindValue(":idStudio", idStudio);
        query.bindValue(":idMovie", idMovie);
        query.exec();
    }

    // Genres
    query.prepare("DELETE FROM genrelinkmovie WHERE idMovie=:idMovie");
    query.bindValue(":idMovie", idMovie);
    query.exec();
    foreach (const QString &genre, movie->genres()) {
        int idGenre = -1;
        query.prepare("SELECT idGenre FROM genre WHERE strGenre=:genre");
        query.bindValue(":genre", genre);
        query.exec();
        if (query.next()) {
            idGenre = query.value(query.record().indexOf("idGenre")).toInt();
        } else {
            query.prepare("INSERT INTO genre(strGenre) VALUES(:genre)");
            query.bindValue(":genre", genre);
            query.exec();
            idGenre = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO genrelinkmovie(idGenre, idMovie) VALUES(:idGenre, :idMovie)");
        query.bindValue(":idGenre", idGenre);
        query.bindValue(":idMovie", idMovie);
        query.exec();
    }

    // Countries
    query.prepare("DELETE FROM countrylinkmovie WHERE idMovie=:idMovie");
    query.bindValue(":idMovie", idMovie);
    query.exec();
    foreach (const QString &country, movie->countries()) {
        int idCountry = -1;
        query.prepare("SELECT idCountry FROM country WHERE strCountry=:country");
        query.bindValue(":country", country);
        query.exec();
        if (query.next()) {
            idCountry = query.value(query.record().indexOf("idCountry")).toInt();
        } else {
            query.prepare("INSERT INTO country(strCountry) VALUES(:country)");
            query.bindValue(":country", country);
            query.exec();
            idCountry = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO countrylinkmovie(idCountry, idMovie) VALUES(:idCountry, :idMovie)");
        query.bindValue(":idCountry", idCountry);
        query.bindValue(":idMovie", idMovie);
        query.exec();
    }

    // Actors
    query.prepare("DELETE FROM actorlinkmovie WHERE idMovie=:idMovie");
    query.bindValue(":idMovie", idMovie);
    query.exec();
    int iOrder = 0;
    foreach (const Actor &actor, movie->actors()) {
        int idActor = -1;
        query.prepare("SELECT idActor FROM actors WHERE strActor=:actor");
        query.bindValue(":actor", actor.name);
        query.exec();
        if (query.next()) {
            idActor = query.value(query.record().indexOf("idActor")).toInt();
        } else {
            query.prepare("INSERT INTO actors(strActor, strThumb) VALUES(:actor, :thumb)");
            query.bindValue(":actor", actor.name);
            query.bindValue(":thumb", actor.thumb);
            query.exec();
            idActor = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO actorlinkmovie(idActor, idMovie, strRole, iOrder) VALUES(:idActor, :idMovie, :role, :order)");
        query.bindValue(":idActor", idActor);
        query.bindValue(":idMovie", idMovie);
        query.bindValue(":role", actor.role);
        query.bindValue(":order", iOrder++);
        query.exec();
    }

    // Set
    query.prepare("DELETE FROM setlinkmovie WHERE idMovie=:idMovie");
    query.bindValue(":idMovie", idMovie);
    query.exec();
    if (!movie->set().isEmpty()) {
        int idSet = -1;
        query.prepare("SELECT idSet FROM sets WHERE strSet=:set");
        query.bindValue(":set", movie->set());
        query.exec();
        if (query.next()) {
            idSet = query.value(query.record().indexOf("idSet")).toInt();
        } else {
            query.prepare("INSERT INTO sets(strSet) VALUES(:set)");
            query.bindValue(":set", movie->set());
            query.exec();
            idSet = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO setlinkmovie(idSet, idMovie) VALUES(:idSet, :idMovie)");
        query.bindValue(":idSet", idSet);
        query.bindValue(":idMovie", idMovie);
        query.exec();
    }

    // save images
    qDebug() << "Saving images";
    QString fileHash = hash(mediaCenterPath(movie->files().at(0)));
    qDebug() << "Hash for file" << movie->files().at(0) << "with mediaCenterPath" << mediaCenterPath(movie->files().at(0)) << "is" << fileHash;
    QString fanartHash = fileHash;
    if (movie->files().count() > 1) {
        qDebug() << "Multiple files";
        QStringList files;
        foreach (const QString &file, movie->files()) {
            qDebug() << "file" << file << "becomes" << mediaCenterPath(file);
            files << mediaCenterPath(file);
        }
        fanartHash = hash(QString("stack://%1").arg(files.join(" , ")));
        qDebug() << "Hash for" << QString("stack://%1").arg(files.join(" , ")) << "is" << fanartHash;
    }

    QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
    QString fanartPath = QString("%1%2Video%2Fanart%2%3.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash);
    qDebug() << "posterPath=" << posterPath << "fanartPath" << fanartPath;
    if (movie->posterImageChanged() && !movie->posterImage()->isNull()) {
        qDebug() << "Movie poster has changed, saving";
        movie->posterImage()->save(posterPath, "jpg", 100);
    }
    if (movie->backdropImageChanged() && !movie->backdropImage()->isNull()) {
        qDebug() << "Movie backdrop has changed, saving";
        movie->backdropImage()->save(fanartPath, "jpg", 100);
    }

    foreach (Actor actor, movie->actors()) {
        if (actor.image.isNull())
            continue;
        QString hashActor = actorHash(actor);
        QString actorThumb = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hashActor.left(1)).arg(hashActor);
        actor.image.save(actorThumb, "jpg", 100);
    }

    return true;
}

/**
 * @brief Loads all movie information except images
 * @param movie The movie to load infos for
 * @return Loading success
 */
bool XbmcSql::loadMovie(Movie *movie)
{
    qDebug() << "Entered, movie=" << movie->name();
    if (movie->files().size() == 0) {
        qWarning() << "Movie has no files";
        return false;
    }
    QFileInfo fi(movie->files().at(0));
    if (!fi.isFile() ) {
        qWarning() << "File" << movie->files().at(0) << "doesn't exist";
        return false;
    }
    movie->clear();
    movie->setChanged(false);

    QSqlQuery query(db());
    QString file = mediaCenterPath(movie->files().at(0));
    qDebug() << "File" << movie->files().at(0) << "becomes" << file;
    if (movie->files().count() == 1) {
        qDebug() << "Movie is a single file movie";
        bool isUnixFile = !file.contains("\\");
        qDebug() << "isUnixFile=" << isUnixFile;
        QStringList fileSplit = (isUnixFile) ? file.split("/") : file.split("\\");
        qDebug() << "fileSplit=" << fileSplit;
        if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0 ||
                                     QString::compare(fileSplit.last(), "index.bdmv", Qt::CaseInsensitive) == 0)) {
            fileSplit.takeLast();
            if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0 ||
                                         QString::compare(fileSplit.last(), "BDMV", Qt::CaseInsensitive) == 0)) {
                fileSplit.takeLast();
            }
            file = (isUnixFile) ? fileSplit.join("/") : fileSplit.join("\\");
            if (!file.endsWith("/") && !file.endsWith("\\"))
                file.append((isUnixFile) ? "/" : "\\");
            qDebug() << "file is now" << file;
        }

        query.prepare("SELECT "
                      "M.idMovie, "
                      "M.c00 AS title, "
                      "M.c01 AS plot, "
                      "M.c03 AS tagline, "
                      "M.c05 AS rating, "
                      "M.c07 AS year, "
                      "M.c08 AS thumbnails, "
                      "M.c10 AS sortTitle, "
                      "M.c11 AS runtime, "
                      "M.c12 AS certification, "
                      "M.c14 AS genres, "
                      "M.c16 AS originalTitle, "
                      "M.c18 AS studios, "
                      "M.c19 AS trailer, "
                      "M.c20 AS fanart, "
                      "M.c21 AS countries, "
                      "F.playCount AS playCount, "
                      "F.lastPlayed AS lastPlayed, "
                      "M.c22 AS filePath "
                      "FROM movie M "
                      "JOIN files F ON M.idFile=F.idFile "
                      "LEFT JOIN setlinkmovie SLM ON SLM.idMovie=M.idMovie "
                      "LEFT JOIN sets S ON S.idSet=SLM.idSet "
                      "WHERE M.c22=:file");
        query.bindValue(":file", file);

    } else {
        qDebug() << "Movie is made of multiple files";
        query.prepare("SELECT "
                      "M.idMovie, "
                      "M.c00 AS title, "
                      "M.c01 AS plot, "
                      "M.c03 AS tagline, "
                      "M.c05 AS rating, "
                      "M.c07 AS year, "
                      "M.c08 AS thumbnails, "
                      "M.c10 AS sortTitle, "
                      "M.c11 AS runtime, "
                      "M.c12 AS certification, "
                      "M.c14 AS genres, "
                      "M.c16 AS originalTitle, "
                      "M.c18 AS studios, "
                      "M.c19 AS trailer, "
                      "M.c20 AS fanart, "
                      "M.c21 AS countries, "
                      "F.playCount AS playCount, "
                      "F.lastPlayed AS lastPlayed, "
                      "M.c22 AS filePath "
                      "FROM movie M "
                      "JOIN files F ON M.idFile=F.idFile "
                      "LEFT JOIN setlinkmovie SLM ON SLM.idMovie=M.idMovie "
                      "LEFT JOIN sets S ON S.idSet=SLM.idSet "
                      "WHERE M.c22 LIKE :file");
        query.bindValue(":file", QString("stack://%%1%").arg(file));
    }
    query.exec();

    if (movie->files().count() == 1) {
        qDebug() << "Single file movie";
        if (!query.next()) {
            qDebug() << "Got no result, trying foldername";
            // Try the folder name instead
            bool isUnixFile = !file.contains("\\");
            QStringList fileSplit = (isUnixFile) ? file.split("/") : file.split("\\");
            fileSplit.takeLast();
            file = (isUnixFile) ? fileSplit.join("/") : fileSplit.join("\\");
            file.append((isUnixFile) ? "/" : "\\");
            qDebug() << "file is now" << file;

            query.clear();
            query.prepare("SELECT "
                          "M.idMovie, "
                          "M.c00 AS title, "
                          "M.c01 AS plot, "
                          "M.c03 AS tagline, "
                          "M.c05 AS rating, "
                          "M.c07 AS year, "
                          "M.c08 AS thumbnails, "
                          "M.c10 AS sortTitle, "
                          "M.c11 AS runtime, "
                          "M.c12 AS certification, "
                          "M.c14 AS genres, "
                          "M.c16 AS originalTitle, "
                          "M.c18 AS studios, "
                          "M.c19 AS trailer, "
                          "M.c20 AS fanart, "
                          "M.c21 AS countries, "
                          "F.playCount AS playCount, "
                          "F.lastPlayed AS lastPlayed, "
                          "M.c22 AS filePath "
                          "FROM movie M "
                          "JOIN files F ON M.idFile=F.idFile "
                          "LEFT JOIN setlinkmovie SLM ON SLM.idMovie=M.idMovie "
                          "LEFT JOIN sets S ON S.idSet=SLM.idSet "
                          "WHERE M.c22=:file");
            query.bindValue(":file", file);
            query.exec();
            if (!query.next()) {
                qDebug() << "No entry found for movie, giving up";
                return false;
            }
        }
    } else {
        qDebug() << "Stacked movie";
        bool found = false;
        while (!found && query.next()) {
            QString path = query.value(query.record().indexOf("filePath")).toString();
            path.replace("stack://", "");
            qDebug() << "Path is now" << path;
            QStringList dbPaths = path.split(" , ", QString::SkipEmptyParts);
            QStringList filePaths;
            foreach (const QString &path, movie->files())
                filePaths << mediaCenterPath(path);
            qSort(filePaths);
            qSort(dbPaths);
            qDebug() << "filePaths=" << filePaths << "dbPaths=" << dbPaths;
            if (dbPaths == filePaths) {
                qDebug() << "Paths match";
                found = true;
            }
        }

        if (!found) {
            qDebug() << "No entry found for movie, giving up";
            return false;
        }
    }

    QSqlRecord record = query.record();
    int idMovie = query.value(record.indexOf("idMovie")).toInt();
    qDebug() << "idMovie=" << idMovie;
    movie->setMediaCenterId(idMovie);
    movie->setName(query.value(record.indexOf("title")).toString());
    movie->setSortTitle(query.value(record.indexOf("sortTitle")).toString());
    movie->setOverview(query.value(record.indexOf("plot")).toString());
    movie->setTagline(query.value(record.indexOf("tagline")).toString());
    movie->setRating(query.value(record.indexOf("rating")).toReal());
    movie->setReleased(QDate(query.value(record.indexOf("year")).toInt(), 1, 1));
    movie->setRuntime(query.value(record.indexOf("runtime")).toInt());
    movie->setCertification(query.value(record.indexOf("certification")).toString());
    movie->setGenres(query.value(record.indexOf("genres")).toString().split(" / "));
    movie->setOriginalName(query.value(record.indexOf("originalTitle")).toString());
    movie->setStudios(query.value(record.indexOf("studios")).toString().split(" / "));
    movie->setTrailer(query.value(record.indexOf("trailer")).toString());
    movie->setCountries(query.value(record.indexOf("countries")).toString().split(" / "));
    movie->setPlayCount(query.value(record.indexOf("playCount")).toInt());
    movie->setLastPlayed(QDateTime::fromString(query.value(record.indexOf("lastPlayed")).toString(), "yyyy-MM-dd HH:mm:ss"));
    movie->setWatched(query.value(record.indexOf("playCount")).toInt() > 0);

    // Posters
    QDomDocument domDoc;
    domDoc.setContent(QString("<thumbnails>%1</thumbnails>").arg(query.value(record.indexOf("thumbnails")).toString()));
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        Poster p;
        p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
        p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
        movie->addPoster(p);
    }

    // Fanart/Backdrops
    domDoc.setContent(query.value(record.indexOf("fanart")).toString());
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        Poster p;
        p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
        p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
        movie->addBackdrop(p);
    }

    // Set
    query.prepare("SELECT S.strSet AS `set` "
                  "FROM setlinkmovie SLM "
                  "JOIN sets S ON SLM.idSet=S.idSet "
                  "WHERE SLM.idMovie=:idMovie");
    query.bindValue(":idMovie", idMovie);
    query.exec();
    if (query.next())
        movie->setSet(query.value(query.record().indexOf("set")).toString());

    // Actors
    query.prepare("SELECT "
                  "ALM.strRole AS role, "
                  "A.strActor AS actor, "
                  "A.strThumb AS thumb "
                  "FROM actorlinkmovie ALM "
                  "JOIN actors A ON A.idActor=ALM.idActor "
                  "WHERE ALM.idMovie=:idMovie "
                  "ORDER BY ALM.iOrder");
    query.bindValue(":idMovie", idMovie);
    query.exec();
    while (query.next()) {
        Actor a;
        a.name = query.value(query.record().indexOf("actor")).toString();
        a.role = query.value(query.record().indexOf("role")).toString();
        a.thumb = query.value(query.record().indexOf("thumb")).toString();
        movie->addActor(a);
    }

    return true;
}

/**
 * @brief Loads images for a movie
 * @param movie The movie to load images for
 */
void XbmcSql::loadMovieImages(Movie *movie)
{
    qDebug() << "Entered, movie=" << movie->name();
    if (movie->files().count() == 0) {
        qWarning() << "Movie has no files";
        return;
    }

    QString fileHash = hash(mediaCenterPath(movie->files().at(0)));
    QString fanartHash = fileHash;
    qDebug() << "First file is" << movie->files().at(0) << "becomes" << mediaCenterPath(movie->files().at(0)) << "hash=" << fileHash;
    if (movie->files().count() > 1) {
        qDebug() << "Stacked files movie";
        QStringList files;
        foreach (const QString &file, movie->files())
            files << mediaCenterPath(file);
        qDebug() << "files=" << files;
        fanartHash = hash(QString("stack://%1").arg(files.join(" , ")));
    }
    qDebug() << "fanartHash=" << fanartHash;

    QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
    QString fanartPath = QString("%1%2Video%2Fanart%2%3.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash);
    QFileInfo posterFi(posterPath);
    QFileInfo fanartFi(fanartPath);
    qDebug() << "posterPath=" << posterPath;
    qDebug() << "fanartPath=" << fanartPath;
    if (posterFi.isFile()) {
        qDebug() << "Trying to load poster" << posterPath;
        movie->posterImage()->load(posterPath);
    }
    if (fanartFi.isFile()) {
        qDebug() << "Trying to load backdrop" << fanartPath;
        movie->backdropImage()->load(fanartPath);
    }

    foreach (Actor *actor, movie->actorsPointer()) {
        if (actor->imageHasChanged)
            continue;
        QString hashActor = actorHash(*actor);
        QString actorThumb = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hashActor.left(1)).arg(hashActor);
        QFileInfo fi(actorThumb);
        if (fi.isFile())
            actor->image.load(actorThumb);
    }
}


/**
 * @brief Saves concert information
 * @param concert The concert to save
 * @return Saving success
 */
bool XbmcSql::saveConcert(Concert *concert)
{
    qDebug() << "Entered, concert=" << concert->name();
    QSqlQuery query(db());

    // get Path ID
    int idPath = -1;
    int mediaCenterId = concert->mediaCenterId();
    qDebug() << "mediaCenterId=" << mediaCenterId;
    if (mediaCenterId != -1) {
        query.prepare("SELECT c14 FROM musicvideo WHERE idMVideo=:idMVideo");
        query.bindValue(":idMVideo", concert->mediaCenterId());
        query.exec();
        if (query.next()) {
            idPath = query.value(0).toInt();
            qDebug() << "Got path id based on idMVideo, idPath=" << idPath;
        }
    }
    QFileInfo fiPath(concertMediaCenterPath(concert->files().at(0)));
    qDebug() << "fiPath=" << fiPath.absoluteFilePath();
    if (idPath == -1) {
        qDebug() << "idPath is -1";
        QString path = fiPath.path();
        qDebug() << "path=" << path;
        QString mediaCenterPath2 = concertMediaCenterPath(concert->files().at(0));
        qDebug() << "mediaCenterPath=" << mediaCenterPath2;
        if (mediaCenterPath2.contains("\\")) {
            qDebug() << "mediaCenterPath contains \\";
            path.replace("/", "\\");
            if (!path.endsWith("\\"))
                path.append("\\");
        } else {
            path.replace("\\", "/");
            if (!path.endsWith("/"))
                path.append("/");
        }
        qDebug() << "path is now" << path;

        query.prepare("SELECT idPath FROM path WHERE strPath=:path");
        query.bindValue(":path", path);
        query.exec();
        qDebug() << query.lastQuery() << ":path=" << path;
        if (query.next()) {
            idPath = query.value(query.record().indexOf("idPath")).toInt();
            qDebug() << "Got path, idPath=" << idPath;
        } else {
            query.prepare("INSERT INTO path(strPath, strContent, strScraper, scanRecursive, useFolderNames, noUpdate, exclude) "
                          "VALUES(:path, 'musicvideos', 'metadata.theconcertdb.org', 0, 0, 1, 0)");
            query.bindValue(":path", path);
            query.exec();
            idPath = query.lastInsertId().toInt();
            qDebug() << "Inserted path" << path << "got idPath=" << idPath;
        }
    }

    // get File ID
    int idMVideo = -1;
    int idFile = -1;
    if (concert->mediaCenterId() != -1) {
        idMVideo = concert->mediaCenterId();
        query.prepare("SELECT idFile FROM musicvideo WHERE idMVideo=:idMVideo");
        query.bindValue(":idMVideo", idMVideo);
        query.exec();
        qDebug() << query.lastQuery() << ":idMVideo=" << idMVideo;
        if (query.next()) {
            idFile = query.value(0).toInt();
            qDebug() << "Got idFile=" << idFile;
        }
    } else {
        if (concert->files().count() == 1) {
            qDebug() << "Concert is single file";
            query.prepare("SELECT idFile, strFilename FROM files WHERE strFilename=:fileName AND idPath=:idPath");
            query.bindValue(":idPath", idPath);
            query.bindValue(":fileName", fiPath.fileName());
            query.exec();
            qDebug() << query.lastQuery() << ":fileName=" << fiPath.fileName() << ":idPath=" << idPath;
        } else {
            qDebug() << "Concert contains multiple files";
            query.prepare(QString("SELECT idFile, strFilename FROM files WHERE strFilename LIKE 'stack://%%1%' AND idPath='%2'").arg(fiPath.fileName().replace("'", "''")).arg(idPath));
            query.exec();
            qDebug() << query.lastQuery();
        }
        if (concert->files().count() == 1) {
            if (query.next()) {
                idFile = query.value(query.record().indexOf("idFile")).toInt();
                qDebug() << "Got (single file) idFile=" << idFile;
            }
        } else {
            while (idFile == -1 && query.next()) {
                qDebug() << "Checking if entry matches all files";
                QString path = query.value(query.record().indexOf("strFilename")).toString();
                path.replace("stack://", "");
                qDebug() << "Path is now" << path;
                QStringList dbPaths = path.split(" , ", QString::SkipEmptyParts);
                QStringList filePaths;
                foreach (const QString &path, concert->files())
                    filePaths << concertMediaCenterPath(path);
                qSort(filePaths);
                qSort(dbPaths);
                qDebug() << "filePaths=" << filePaths;
                qDebug() << "dbPaths=" << dbPaths;
                if (dbPaths == filePaths) {
                    idFile = query.value(query.record().indexOf("idFile")).toInt();
                    qDebug() << "FilePaths and dbPaths match, idFile=" << idFile;
                }
            }
        }
    }

    // update file data and get concert id, or insert file
    if (idFile != -1) {
        qDebug() << "We have idFile=" << idFile;
        query.prepare("SELECT idMVideo FROM musicvideo WHERE idFile=:idFile");
        query.bindValue(":idFile", idFile);
        query.exec();
        if (query.next()) {
            idMVideo = query.value(query.record().indexOf("idMVideo")).toInt();
            qDebug() << "Got idMVideo=" << idMVideo;
        }
        query.prepare("UPDATE files SET playCount=:playCount, lastPlayed=:lastPlayed WHERE idFile=:idFile");
        query.bindValue(":idFile", idFile);
        if (concert->playcount() == 0)
            query.bindValue(":playCount", QVariant(QVariant::Int));
        else
            query.bindValue(":playCount", concert->playcount());
        query.bindValue(":lastPlayed", concert->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
        query.exec();
    } else {
        qDebug() << "Inserting new file";
        QString filename;
        if (concert->files().count() == 1) {
            QFileInfo fiFile(concert->files().at(0));
            filename = fiFile.fileName();
        } else {
            QStringList files;
            foreach (const QString &file, concert->files())
                files << concertMediaCenterPath(file);
            filename = QString("stack://%1").arg(files.join(" , "));
        }
        query.prepare("INSERT INTO files(idPath, strFilename, playCount, lastPlayed) VALUES(:idPath, :filename, :playCount, :lastPlayed)");
        query.bindValue(":idPath", idPath);
        query.bindValue(":filename", filename);
        if (concert->playcount() == 0)
            query.bindValue(":playCount", QVariant(QVariant::Int));
        else
            query.bindValue(":playCount", concert->playcount());
        query.bindValue(":lastPlayed", concert->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
        query.exec();
        qDebug() << query.lastQuery() << ":idPath=" << idPath << ":filename=" << filename;
        idFile = query.lastInsertId().toInt();
        qDebug() << "idFile=" << idFile;
    }

    // create xml data for thumbnails and fanart
    QByteArray thumbnails;
    QXmlStreamWriter xml(&thumbnails);

    foreach (const Poster &poster, concert->posters()) {
        xml.writeStartElement("thumb");
        xml.writeAttribute("preview", poster.thumbUrl.toString());
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
    }

    // update concert info or insert new concert
    if (idMVideo != -1) {
        qDebug() << "We have idMVideo, updating...";
        query.prepare("UPDATE musicvideo SET "
                      "c00=:title, "
                      "c01=:thumbnails, "
                      "c04=:runtime, "
                      "c07=:year, "
                      "c08=:plot, "
                      "c11=:genres "
                      "WHERE idMVideo=:idMVideo ");
        query.bindValue(":idMVideo", idMVideo);
        query.bindValue(":title", concert->name());
        query.bindValue(":plot", concert->overview());
        query.bindValue(":year", concert->released().toString("yyyy"));
        query.bindValue(":thumbnails", QString(thumbnails));
        if (concert->runtime() == 0)
            query.bindValue(":runtime", QVariant(QVariant::Int));
        else
            query.bindValue(":runtime", concert->runtime());
        query.bindValue(":genres", concert->genres().join(" / "));
        query.exec();
    } else {
        qDebug() << "No idMVideo, inserting a new concert";
        QString file;
        if (concert->files().count() == 1) {
            qDebug() << "It's a single file" << concert->files().at(0);
            file = concertMediaCenterPath(concert->files().at(0));
            qDebug() << "mediaCenterPath is " << file;
            bool isUnixFile = !file.contains("\\");
            qDebug() << "isUnixFile=" << isUnixFile;
            QStringList fileSplit = (isUnixFile) ? file.split("/", QString::SkipEmptyParts) : file.split("\\", QString::SkipEmptyParts);
            qDebug() << "fileSplit=" << fileSplit;
            if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0 ||
                                         QString::compare(fileSplit.last(), "index.bdmv", Qt::CaseInsensitive) == 0)) {
                fileSplit.takeLast();
                if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0 ||
                                             QString::compare(fileSplit.last(), "BDMV", Qt::CaseInsensitive) == 0)) {
                    fileSplit.takeLast();
                }
                file = (isUnixFile) ? fileSplit.join("/") : fileSplit.join("\\");
                file.prepend((isUnixFile) ? "/" : "\\");
                file.append((isUnixFile) ? "/" : "\\");
            }
        } else {
            QStringList files;
            foreach (const QString &mFile, concert->files()) {
                qDebug() << mFile << "becomes" << concertMediaCenterPath(mFile);
                files << concertMediaCenterPath(mFile);
            }
            file = "stack://" + files.join(" , ");
        }
        qDebug() << "file=" << file;

        query.prepare("SELECT "
                      "M.idMVideo, "
                      "M.c00 AS title, "
                      "M.c01 AS thumbnails, "
                      "M.c04 AS runtime, "
                      "M.c07 AS year, "
                      "M.c08 AS plot, "
                      "M.c11 AS genres, "
                      "M.c13 AS filePath "
                      "F.playCount AS playCount, "
                      "F.lastPlayed AS lastPlayed "
                      "FROM musicvideo M "
                      "JOIN files F ON M.idFile=F.idFile "
                      "WHERE M.c13=:file");



        query.prepare("INSERT INTO musicvideo(idFile, c00, c01, c04, c07, c08, c11, c13, c14) "
                      "VALUES(:idFile, :title, :thumbnails, :runtime, :year, :plot, :genres, :filePath, :idPath)");
        query.bindValue(":idFile", idFile);
        query.bindValue(":title", concert->name());
        query.bindValue(":thumbnails", QString(thumbnails));
        if (concert->runtime() == 0)
            query.bindValue(":runtime", QVariant(QVariant::Int));
        else
            query.bindValue(":runtime", concert->runtime());
        query.bindValue(":year", concert->released().toString("yyyy"));
        query.bindValue(":plot", concert->overview());
        query.bindValue(":genres", concert->genres().join(" / "));
        query.bindValue(":filePath", file);
        query.bindValue(":idPath", idPath);
        query.exec();
        idMVideo = query.lastInsertId().toInt();
        qDebug() << "idMVideo=" << idMVideo;
    }

    // Genres
    query.prepare("DELETE FROM genrelinkmusicvideo WHERE idMVideo=:idMVideo");
    query.bindValue(":idMVideo", idMVideo);
    query.exec();
    foreach (const QString &genre, concert->genres()) {
        int idGenre = -1;
        query.prepare("SELECT idGenre FROM genre WHERE strGenre=:genre");
        query.bindValue(":genre", genre);
        query.exec();
        if (query.next()) {
            idGenre = query.value(query.record().indexOf("idGenre")).toInt();
        } else {
            query.prepare("INSERT INTO genre(strGenre) VALUES(:genre)");
            query.bindValue(":genre", genre);
            query.exec();
            idGenre = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO genrelinkmusicvideo(idGenre, idMVideo) VALUES(:idGenre, :idMVideo)");
        query.bindValue(":idGenre", idGenre);
        query.bindValue(":idMVideo", idMVideo);
        query.exec();
    }

    // save images
    qDebug() << "Saving images";
    QString fileHash = hash(concertMediaCenterPath(concert->files().at(0)));
    qDebug() << "Hash for file" << concert->files().at(0) << "with mediaCenterPath" << concertMediaCenterPath(concert->files().at(0)) << "is" << fileHash;
    QString fanartHash = fileHash;
    if (concert->files().count() > 1) {
        qDebug() << "Multiple files";
        QStringList files;
        foreach (const QString &file, concert->files()) {
            qDebug() << "file" << file << "becomes" << concertMediaCenterPath(file);
            files << concertMediaCenterPath(file);
        }
        fanartHash = hash(QString("stack://%1").arg(files.join(" , ")));
        qDebug() << "Hash for" << QString("stack://%1").arg(files.join(" , ")) << "is" << fanartHash;
    }

    QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
    QString fanartPath = QString("%1%2Video%2Fanart%2%3.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash);
    qDebug() << "posterPath=" << posterPath << "fanartPath" << fanartPath;
    if (concert->posterImageChanged() && !concert->posterImage()->isNull()) {
        qDebug() << "Concert poster has changed, saving";
        concert->posterImage()->save(posterPath, "jpg", 100);
    }
    if (concert->backdropImageChanged() && !concert->backdropImage()->isNull()) {
        qDebug() << "Concert backdrop has changed, saving";
        concert->backdropImage()->save(fanartPath, "jpg", 100);
    }

    return true;
}

/**
 * @brief Loads all concert information except images
 * @param concert The concert to load infos for
 * @return Loading success
 */
bool XbmcSql::loadConcert(Concert *concert)
{
    qDebug() << "Entered, concert=" << concert->name();
    if (concert->files().size() == 0) {
        qWarning() << "Concert has no files";
        return false;
    }
    QFileInfo fi(concert->files().at(0));
    if (!fi.isFile() ) {
        qWarning() << "File" << concert->files().at(0) << "doesn't exist";
        return false;
    }
    concert->clear();
    concert->setChanged(false);

    QSqlQuery query(db());
    QString file = concertMediaCenterPath(concert->files().at(0));
    qDebug() << "File" << concert->files().at(0) << "becomes" << file;
    if (concert->files().count() == 1) {
        qDebug() << "Concert is a single file";
        bool isUnixFile = !file.contains("\\");
        qDebug() << "isUnixFile=" << isUnixFile;
        QStringList fileSplit = (isUnixFile) ? file.split("/") : file.split("\\");
        qDebug() << "fileSplit=" << fileSplit;
        if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0 ||
                                     QString::compare(fileSplit.last(), "index.bdmv", Qt::CaseInsensitive) == 0)) {
            fileSplit.takeLast();
            if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0 ||
                                         QString::compare(fileSplit.last(), "BDMV", Qt::CaseInsensitive) == 0)) {
                fileSplit.takeLast();
            }
            file = (isUnixFile) ? fileSplit.join("/") : fileSplit.join("\\");
            if (!file.endsWith("/") && !file.endsWith("\\"))
                file.append((isUnixFile) ? "/" : "\\");
            qDebug() << "file is now" << file;
        }

        query.prepare("SELECT "
                      "M.idMVideo, "
                      "M.c00 AS title, "
                      "M.c01 AS thumbnails, "
                      "M.c04 AS runtime, "
                      "M.c07 AS year, "
                      "M.c08 AS plot, "
                      "M.c11 AS genres, "
                      "M.c13 AS filePath, "
                      "F.playCount AS playCount, "
                      "F.lastPlayed AS lastPlayed "
                      "FROM musicvideo M "
                      "JOIN files F ON M.idFile=F.idFile "
                      "WHERE M.c13=:file");
        query.bindValue(":file", file);
    } else {
        qDebug() << "Concert is made of multiple files";
        query.prepare("SELECT "
                      "M.idMVideo, "
                      "M.c00 AS title, "
                      "M.c01 AS thumbnails, "
                      "M.c04 AS runtime, "
                      "M.c07 AS year, "
                      "M.c08 AS plot, "
                      "M.c11 AS genres, "
                      "M.c13 AS filePath, "
                      "F.playCount AS playCount, "
                      "F.lastPlayed AS lastPlayed "
                      "FROM musicvideo M "
                      "JOIN files F ON M.idFile=F.idFile "
                      "WHERE M.c13 LIKE :file");
        query.bindValue(":file", QString("stack://%%1%").arg(file));
    }
    query.exec();
    qDebug() << query.lastQuery();

    if (concert->files().count() == 1) {
        qDebug() << "Single file concert";
        if (!query.next()) {
            qDebug() << "Got no result, trying foldername";
            // Try the folder name instead
            bool isUnixFile = !file.contains("\\");
            QStringList fileSplit = (isUnixFile) ? file.split("/") : file.split("\\");
            fileSplit.takeLast();
            file = (isUnixFile) ? fileSplit.join("/") : fileSplit.join("\\");
            file.append((isUnixFile) ? "/" : "\\");
            qDebug() << "file is now" << file;

            query.clear();
            query.prepare("SELECT "
                          "M.idMVideo, "
                          "M.c00 AS title, "
                          "M.c01 AS thumbnails, "
                          "M.c04 AS runtime, "
                          "M.c07 AS year, "
                          "M.c08 AS plot, "
                          "M.c11 AS genres, "
                          "M.c13 AS filePath, "
                          "F.playCount AS playCount, "
                          "F.lastPlayed AS lastPlayed "
                          "FROM musicvideo M "
                          "JOIN files F ON M.idFile=F.idFile "
                          "WHERE M.c13=:file");
            query.bindValue(":file", file);
            query.exec();
            if (!query.next()) {
                qDebug() << "No entry found for concert, giving up";
                return false;
            }
        }
    } else {
        qDebug() << "Stacked concert";
        bool found = false;
        while (!found && query.next()) {
            QString path = query.value(query.record().indexOf("filePath")).toString();
            path.replace("stack://", "");
            qDebug() << "Path is now" << path;
            QStringList dbPaths = path.split(" , ", QString::SkipEmptyParts);
            QStringList filePaths;
            foreach (const QString &path, concert->files())
                filePaths << concertMediaCenterPath(path);
            qSort(filePaths);
            qSort(dbPaths);
            qDebug() << "filePaths=" << filePaths << "dbPaths=" << dbPaths;
            if (dbPaths == filePaths) {
                qDebug() << "Paths match";
                found = true;
            }
        }

        if (!found) {
            qDebug() << "No entry found for concert, giving up";
            return false;
        }
    }

    QSqlRecord record = query.record();
    int idMVideo = query.value(record.indexOf("idMVideo")).toInt();
    qDebug() << "idMVideo=" << idMVideo;
    concert->setMediaCenterId(idMVideo);
    concert->setName(query.value(record.indexOf("title")).toString());
    concert->setOverview(query.value(record.indexOf("plot")).toString());
    concert->setReleased(QDate(query.value(record.indexOf("year")).toInt(), 1, 1));
    concert->setRuntime(query.value(record.indexOf("runtime")).toInt());
    concert->setGenres(query.value(record.indexOf("genres")).toString().split(" / "));
    concert->setPlayCount(query.value(record.indexOf("playCount")).toInt());
    concert->setLastPlayed(QDateTime::fromString(query.value(record.indexOf("lastPlayed")).toString(), "yyyy-MM-dd HH:mm:ss"));
    concert->setWatched(query.value(record.indexOf("playCount")).toInt() > 0);

    // Posters
    QDomDocument domDoc;
    domDoc.setContent(QString("<thumbnails>%1</thumbnails>").arg(query.value(record.indexOf("thumbnails")).toString()));
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        Poster p;
        p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
        p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
        concert->addPoster(p);
    }

    return true;
}

/**
 * @brief Loads images for a concert
 * @param concert The concert to load images for
 */
void XbmcSql::loadConcertImages(Concert *concert)
{
    qDebug() << "Entered, concert=" << concert->name();
    if (concert->files().count() == 0) {
        qWarning() << "Concert has no files";
        return;
    }

    QString fileHash = hash(concertMediaCenterPath(concert->files().at(0)));
    QString fanartHash = fileHash;
    qDebug() << "First file is" << concert->files().at(0) << "becomes" << concertMediaCenterPath(concert->files().at(0)) << "hash=" << fileHash;
    if (concert->files().count() > 1) {
        qDebug() << "Stacked files concert";
        QStringList files;
        foreach (const QString &file, concert->files())
            files << concertMediaCenterPath(file);
        qDebug() << "files=" << files;
        fanartHash = hash(QString("stack://%1").arg(files.join(" , ")));
    }
    qDebug() << "fanartHash=" << fanartHash;

    QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
    QString fanartPath = QString("%1%2Video%2Fanart%2%3.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash);
    QFileInfo posterFi(posterPath);
    QFileInfo fanartFi(fanartPath);
    qDebug() << "posterPath=" << posterPath;
    qDebug() << "fanartPath=" << fanartPath;
    if (posterFi.isFile()) {
        qDebug() << "Trying to load poster" << posterPath;
        concert->posterImage()->load(posterPath);
    }
    if (fanartFi.isFile()) {
        qDebug() << "Trying to load backdrop" << fanartPath;
        concert->backdropImage()->load(fanartPath);
    }
}

/**
 * @brief Exports the database. Disabled till rework
 * @param movies
 * @param shows
 * @param exportPath
 * @param pathSearch
 * @param pathReplace
 * @todo: Remove or reimplement (Export)
 */
void XbmcSql::exportDatabase(QList<Movie *> movies, QList<TvShow *> shows, QString exportPath, QString pathSearch, QString pathReplace)
{
    Q_UNUSED(movies);
    Q_UNUSED(shows);
    Q_UNUSED(exportPath);
    Q_UNUSED(pathSearch);
    Q_UNUSED(pathReplace);
}

/**
 * @brief Loads infos for a tv show (except images)
 * @param show Show to load infos for
 * @return Loading success
 */
bool XbmcSql::loadTvShow(TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    if (show->dir().isEmpty()) {
        qWarning() << "Show has no dir";
        return false;
    }
    show->clear();

    QString path = tvShowMediaCenterPath(show->dir());
    qDebug() << "Show dir" << show->dir() << "becomes" << path;
    if (path.contains("\\") && !path.endsWith("\\"))
        path.append("\\");
    else if (path.contains("/") && !path.endsWith("/"))
        path.append("/");

    qDebug() << "Path is now" << path;

    QSqlQuery query(db());
    query.prepare("SELECT "
                  "S.idShow, "
                  "S.c00 AS title, "
                  "S.c01 AS plot, "
                  "S.c04 AS rating, "
                  "S.c05 AS firstAired, "
                  "S.c06 AS thumbnails, "
                  "S.c08 AS genres, "
                  "S.c11 AS fanart, "
                  "S.c13 AS certification, "
                  "S.c14 AS network, "
                  "S.c16 AS folder "
                  "FROM tvshow S "
                  "WHERE S.c16=:path");
    query.bindValue(":path", path);
    query.exec();
    if (!query.next()) {
        qDebug() << "Found no entry for this path, giving up";
        return false;
    }

    QSqlRecord record = query.record();
    int idShow = query.value(record.indexOf("idShow")).toInt();
    qDebug() << "idShow=" << idShow;
    show->setMediaCenterPath(query.value(record.indexOf("folder")).toString());
    show->setName(query.value(record.indexOf("title")).toString());
    show->setOverview(query.value(record.indexOf("plot")).toString());
    show->setRating(query.value(record.indexOf("rating")).toReal());
    show->setFirstAired(QDate::fromString(query.value(record.indexOf("firstAired")).toString(), "yyyy-MM-dd"));
    show->setGenres(query.value(record.indexOf("genres")).toString().split(" / "));
    show->setCertification(query.value(record.indexOf("certification")).toString());
    show->setNetwork(query.value(record.indexOf("network")).toString());

    // Posters
    QDomDocument domDoc;
    domDoc.setContent(QString("<thumbnails>%1</thumbnails>").arg(query.value(record.indexOf("thumbnails")).toString()));
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        QDomElement elem = domDoc.elementsByTagName("thumb").at(i).toElement();
        Poster p;
        p.originalUrl = QUrl(elem.text());
        p.thumbUrl = QUrl(elem.text());
        if (elem.hasAttribute("type") && elem.attribute("type") == "season") {
            int season = elem.attribute("season").toInt();
            if (season >= 0)
                show->addSeasonPoster(season, p);
        } else {
            show->addPoster(p);
        }
    }

    // Fanart/Backdrops
    domDoc.setContent(query.value(record.indexOf("fanart")).toString());
    for (int i=0, n=domDoc.elementsByTagName("thumb").size() ; i<n ; i++) {
        Poster p;
        p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
        p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
        show->addBackdrop(p);
    }

    // Actors
    query.prepare("SELECT "
                  "ALS.strRole AS role, "
                  "A.strActor AS actor, "
                  "A.strThumb AS thumb "
                  "FROM actorlinktvshow ALS "
                  "JOIN actors A ON A.idActor=ALS.idActor "
                  "WHERE ALS.idShow=:idShow "
                  "ORDER BY ALS.iOrder");
    query.bindValue(":idShow", idShow);
    query.exec();
    while (query.next()) {
        Actor a;
        a.name = query.value(query.record().indexOf("actor")).toString();
        a.role = query.value(query.record().indexOf("role")).toString();
        a.thumb = query.value(query.record().indexOf("thumb")).toString();
        show->addActor(a);
    }

    return true;
}

/**
 * @brief Loads images for a tv show
 * @param show Show to load images for
 */
void XbmcSql::loadTvShowImages(TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    // English
    QString fileHash = hash(QString("season%1* All Seasons").arg(show->mediaCenterPath()));
    qDebug() << "Hash for" << QString("season%1* All Seasons").arg(show->mediaCenterPath()) << "is" << fileHash;
    QFileInfo posterFi(QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash));
    if (posterFi.isFile()) {
        qDebug() << "Trying to load poster" << posterFi.absoluteFilePath();
        show->posterImage()->load(posterFi.absoluteFilePath());
    } else {
        // German
        fileHash = hash(QString("season%1* Alle Staffeln").arg(show->mediaCenterPath()));
        qDebug() << "Hash for" << QString("season%1* Alle Staffeln").arg(show->mediaCenterPath()) << "is" << fileHash;
        posterFi.setFile(QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash));
        if (posterFi.isFile()) {
            qDebug() << "Trying to load" << posterFi.absoluteFilePath();
            show->posterImage()->load(posterFi.absoluteFilePath());
        }
    }

    QString fanartHash = hash(show->mediaCenterPath());
    qDebug() << "Hash for path" << show->mediaCenterPath() << "is" << fanartHash;
    QFileInfo backdropFi(QString("%1%2Video%2Fanart%2%3.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash));
    if (backdropFi.isFile()) {
        qDebug() << "Trying to load" << backdropFi.absoluteFilePath();
        show->backdropImage()->load(backdropFi.absoluteFilePath());
    }

    QString bannerHash = hash(show->mediaCenterPath());
    QFileInfo bannerFi(QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(bannerHash.left(1)).arg(bannerHash));
    if (bannerFi.isFile()) {
        qDebug() << "Trying to load" << bannerFi.absoluteFilePath();
        show->bannerImage()->load(bannerFi.absoluteFilePath());
    }

    foreach (int season, show->seasons()) {
        // English
        QString seasonHash = hash(QString("season%1Season %2").arg(show->mediaCenterPath()).arg(season));
        QFileInfo seasonFi(QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(seasonHash.left(1)).arg(seasonHash));
        if (seasonFi.isFile()) {
            show->seasonPosterImage(season)->load(seasonFi.absoluteFilePath());
        } else {
            // German
            seasonHash = hash(QString("season%1Staffel %2").arg(show->mediaCenterPath()).arg(season));
            seasonFi.setFile(QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(seasonHash.left(1)).arg(seasonHash));
            if (seasonFi.isFile())
                show->seasonPosterImage(season)->load(seasonFi.absoluteFilePath());
        }
    }

    foreach (Actor *actor, show->actorsPointer()) {
        if (actor->imageHasChanged)
            continue;
        QString hashActor = actorHash(*actor);
        QString actorThumb = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hashActor.left(1)).arg(hashActor);
        QFileInfo fi(actorThumb);
        if (fi.isFile())
            actor->image.load(actorThumb);
    }
}

/**
 * @brief Loads all data (except images) for a tv show episode
 * @param episode Episode to load infos for
 * @return Loading success
 */
bool XbmcSql::loadTvShowEpisode(TvShowEpisode *episode)
{
    qDebug() << "Entered, episode=" << episode->name();
    if (episode->files().count() == 0) {
        qWarning() << "Episode has no files";
        return false;
    }
    episode->clear();

    QString sqlWhereFile;
    QString file = tvShowMediaCenterPath(episode->files().at(0));
    qDebug() << "First file" << episode->files().at(0) << "becomes" << file;
    if (m_isMySQL)
        file.replace("\\", "\\\\");
    else
        file.replace("\\", "\\\\\\\\");
    file.replace("'", "''");
    qDebug() << "file is now" << file;
    if (episode->files().count() == 1) {
        qDebug() << "Episode is one file";
        sqlWhereFile = QString("E.c18='%1'").arg(file);
    } else {
        qDebug() << "Episode is multifile";
        sqlWhereFile = QString("E.c18 LIKE 'stack://%%1%'").arg(file);
    }
    qDebug() << "sqlWhereFile=" << sqlWhereFile;

    QSqlQuery query(db());
    query.prepare(QString("SELECT "
                  "E.idEpisode, "
                  "E.c18 AS filePath, "
                  "E.c00 AS title, "
                  "E.c01 AS plot, "
                  "E.c03 AS rating, "
                  "E.c04 AS writer, "
                  "E.c05 AS firstAired, "
                  "E.c06 AS thumbnails, "
                  "E.c09 AS length, "
                  "E.c10 AS director, "
                  "E.c12 AS season, "
                  "E.c13 AS episode, "
                  "E.c14 AS originalTitle, "
                  "F.playCount AS playCount, "
                  "F.lastPlayed AS lastPlayed "
                  "FROM episode E "
                  "JOIN files F ON E.idFile=F.idFile "
                  "WHERE %1").arg(sqlWhereFile));
    query.exec();

    if (episode->files().count() == 1) {
        qDebug() << "Single file";
        if (!query.next()) {
            qDebug() << "No record found, giving up";
            return false;
        }
    } else {
        qDebug() << "Stacked files";
        bool found = false;
        while (!found && query.next()) {
            QString path = query.value(query.record().indexOf("filePath")).toString();
            path.replace("stack://", "");
            QStringList dbPaths = path.split(" , ", QString::SkipEmptyParts);
            QStringList filePaths;
            foreach (const QString &path, episode->files())
                filePaths << mediaCenterPath(path);
            qSort(filePaths);
            qSort(dbPaths);
            qDebug() << "filePaths=" << filePaths << "dbPaths=" << dbPaths;
            if (dbPaths == filePaths) {
                qDebug() << "Paths match";
                found = true;
            }
        }

        if (!found) {
            qDebug() << "Nothing found, giving up";
            return false;
        }
    }

    QSqlRecord record = query.record();
    episode->setName(query.value(record.indexOf("title")).toString());
    episode->setOverview(query.value(record.indexOf("plot")).toString());
    episode->setRating(query.value(record.indexOf("rating")).toReal());
    episode->setFirstAired(QDate::fromString(query.value(record.indexOf("firstAired")).toString(), "yyyy-MM-dd"));
    episode->setPlayCount(query.value(record.indexOf("playCount")).toInt());
    episode->setLastPlayed(QDateTime::fromString(query.value(record.indexOf("lastPlayed")).toString(), "yyyy-MM-dd HH:mm:ss"));
    episode->setSeason(query.value(record.indexOf("season")).toInt());
    episode->setEpisode(query.value(record.indexOf("episode")).toInt());
    episode->setWriters(query.value(record.indexOf("writer")).toString().split(" / "));
    episode->setDirectors(query.value(record.indexOf("director")).toString().split(" / "));

    // Thumbnails
    QDomDocument domDoc;
    domDoc.setContent(QString("<thumbnails>%1</thumbnails>").arg(query.value(record.indexOf("thumbnails")).toString()));
    if (domDoc.elementsByTagName("thumb").count() > 0) {
        episode->setThumbnail(QUrl(domDoc.elementsByTagName("thumb").at(0).toElement().text()));
    }

    return true;
}

/**
 * @brief Loads image for an episode
 * @param episode Episode to load images for
 */
void XbmcSql::loadTvShowEpisodeImages(TvShowEpisode *episode)
{
    qDebug() << "Entered, episode=" << episode->name();
    if (episode->files().count() == 0) {
        qWarning() << "Episode has no files";
        return;
    }

    QString fileHash = hash(tvShowMediaCenterPath(episode->files().at(0)));
    qDebug() << "First file" << episode->files().at(0) << "becomes" << tvShowMediaCenterPath(episode->files().at(0)) << "with hash" << fileHash;
    QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
    QFileInfo posterFi(posterPath);
    if (posterFi.isFile()) {
        qDebug() << "Trying to load" << posterPath;
        episode->thumbnailImage()->load(posterPath);
    }
}

/**
 * @brief Saves a tv show (including infos, poster, backdrops, season images and actors)
 * @param show Show to save
 * @return Saving success
 */
bool XbmcSql::saveTvShow(TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    if (show->dir().isEmpty()) {
        qWarning() << "Show has no dir";
        return false;
    }

    QSqlQuery query(db());

    // get Path ID
    int idPath = -1;
    QString path = show->mediaCenterPath();
    qDebug() << "path=" << path;
    if (path.isEmpty()) {
        path = tvShowMediaCenterPath(show->dir());
        qDebug() << show->dir() << "becomes" << path;
        if (path.contains("\\") && !path.endsWith("\\"))
            path.append("\\");
        else if (path.contains("/") && !path.endsWith("/"))
            path.append("/");
        qDebug() << "path is now" << path;
    }
    query.prepare("SELECT idPath FROM path WHERE strPath=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idPath = query.value(query.record().indexOf("idPath")).toInt();
        qDebug() << "Path found, idPath=" << idPath;
    } else {
        query.prepare("INSERT INTO path(strPath, strContent, strScraper, scanRecursive, useFolderNames, noUpdate, exclude) "
                      "VALUES(:path, 'tvshows', 'metadata.tvdb.com', 0, 1, 1, 0)");
        query.bindValue(":path", path);
        query.exec();
        idPath = query.lastInsertId().toInt();
        qDebug() << "Path not found, inserting. idPath=" << idPath;
    }

    int idShow = -1;
    query.prepare("SELECT idShow FROM tvshow WHERE c16=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idShow = query.value(query.record().indexOf("idShow")).toInt();
        qDebug() << "Got idShow=" << idShow;
    }

    // create xml data for thumbnails and fanart
    QByteArray thumbnails;
    QByteArray fanart;
    QXmlStreamWriter xml(&thumbnails);
    QXmlStreamWriter xml2(&fanart);

    foreach (const Poster &poster, show->posters()) {
        xml.writeStartElement("thumb");
        xml.writeAttribute("preview", poster.thumbUrl.toString());
        xml.writeCharacters(poster.originalUrl.toString());
        xml.writeEndElement();
    }
    xml2.writeStartElement("fanart");
    foreach (const Poster &poster, show->backdrops()) {
        xml2.writeStartElement("thumb");
        xml2.writeAttribute("preview", poster.thumbUrl.toString());
        xml2.writeCharacters(poster.originalUrl.toString());
        xml2.writeEndElement();
    }
    xml2.writeEndElement();

    if (idShow != -1) {
        qDebug() << "Show exists, updating";
        query.prepare("UPDATE tvshow SET "
                      "c00=:title, "
                      "c01=:plot, "
                      "c04=:rating, "
                      "c05=:firstAired, "
                      "c06=:thumbnails, "
                      "c08=:genres, "
                      "c11=:fanart, "
                      "c13=:certification, "
                      "c14=:network "
                      "WHERE idShow=:idShow");
        query.bindValue(":idShow", idShow);
        query.bindValue(":title", show->name());
        query.bindValue(":plot", show->overview());
        query.bindValue(":rating", show->rating());
        query.bindValue(":firstAired", show->firstAired().toString("yyyy-MM-dd"));
        query.bindValue(":thumbnails", thumbnails);
        query.bindValue(":genres", show->genres().join(" / "));
        query.bindValue(":fanart", fanart);
        query.bindValue(":certification", show->certification());
        query.bindValue(":network", show->network());
        query.exec();
    } else {
        qDebug() << "Inserting new show";
        query.prepare("INSERT INTO tvshow(c00, c01, c04, c05, c06, c08, c11, c13, c14, c16, c17) "
                      "VALUES(:title, :plot, :rating, :firstAired, :thumbnails, :genres, :fanart, :certification, :network, :path, :idPath)");
        query.bindValue(":title", show->name());
        query.bindValue(":plot", show->overview());
        query.bindValue(":rating", show->rating());
        query.bindValue(":firstAired", show->firstAired().toString("yyyy-MM-dd"));
        query.bindValue(":thumbnails", thumbnails);
        query.bindValue(":genres", show->genres().join(" / "));
        query.bindValue(":fanart", fanart);
        query.bindValue(":certification", show->certification());
        query.bindValue(":network", show->network());
        query.bindValue(":path", path);
        query.bindValue(":idPath", idPath);
        query.exec();
        idShow = query.lastInsertId().toInt();
        qDebug() << "idShow is now" << idShow;
    }

    // Studios
    query.prepare("DELETE FROM studiolinktvshow WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();
    int idStudio = -1;
    query.prepare("SELECT idStudio FROM studio WHERE strStudio=:studio");
    query.bindValue(":studio", show->network());
    query.exec();
    if (query.next()) {
        idStudio = query.value(query.record().indexOf("idStudio")).toInt();
    } else {
        query.prepare("INSERT INTO studio(strStudio) VALUES(:studio)");
        query.bindValue(":studio", show->network());
        query.exec();
        idStudio = query.lastInsertId().toInt();
    }
    query.prepare("INSERT INTO studiolinktvshow(idStudio, idShow) VALUES(:idStudio, :idShow)");
    query.bindValue(":idStudio", idStudio);
    query.bindValue(":idShow", idShow);
    query.exec();

    // Genres
    query.prepare("DELETE FROM genrelinktvshow WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();
    foreach (const QString &genre, show->genres()) {
        int idGenre = -1;
        query.prepare("SELECT idGenre FROM genre WHERE strGenre=:genre");
        query.bindValue(":genre", genre);
        query.exec();
        if (query.next()) {
            idGenre = query.value(query.record().indexOf("idGenre")).toInt();
        } else {
            query.prepare("INSERT INTO genre(strGenre) VALUES(:genre)");
            query.bindValue(":genre", genre);
            query.exec();
            idGenre = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO genrelinktvshow(idGenre, idShow) VALUES(:idGenre, :idShow)");
        query.bindValue(":idGenre", idGenre);
        query.bindValue(":idShow", idShow);
        query.exec();
    }

    // Actors
    query.prepare("DELETE FROM actorlinktvshow WHERE idShow=:idShow");
    query.bindValue(":idMovie", idShow);
    query.exec();
    int iOrder = 0;
    foreach (const Actor &actor, show->actors()) {
        int idActor = -1;
        query.prepare("SELECT idActor FROM actors WHERE strActor=:actor");
        query.bindValue(":actor", actor.name);
        query.exec();
        if (query.next()) {
            idActor = query.value(query.record().indexOf("idActor")).toInt();
        } else {
            query.prepare("INSERT INTO actors(strActor, strThumb) VALUES(:actor, :thumb)");
            query.bindValue(":actor", actor.name);
            query.bindValue(":thumb", actor.thumb);
            query.exec();
            idActor = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO actorlinktvshow(idActor, idShow, strRole, iOrder) VALUES(:idActor, :idShow, :role, :order)");
        query.bindValue(":idActor", idActor);
        query.bindValue(":idShow", idShow);
        query.bindValue(":role", actor.role);
        query.bindValue(":order", iOrder++);
        query.exec();
    }

    // Link TvShow -> Path
    query.prepare("SELECT COUNT(*) FROM tvshowlinkpath WHERE idShow=:idShow AND idPath=:idPath");
    query.bindValue(":idShow", idShow);
    query.bindValue(":idPath", idPath);
    query.exec();
    query.next();
    if (query.value(0).toInt() == 0) {
        query.prepare("INSERT INTO tvshowlinkpath(idShow, idPath) VALUES(:idShow, :idPath)");
        query.bindValue(":idShow", idShow);
        query.bindValue(":idPath", idPath);
        query.exec();
    }

    // save images
    // all seasons poster
    if (show->posterImageChanged() && !show->posterImage()->isNull()) {
        qDebug() << "Poster image has changed";
        QString fileHash = hash(path);
        qDebug() << "Hash for" << path << "is" << fileHash;
        QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
        qDebug() << "Trying to save poster to" << posterPath;
        show->posterImage()->save(posterPath, "jpg", 100);

        // poster.jpg
        fileHash = hash(path + "poster.jpg");
        qDebug() << "(poster.jpg) fileHash=" << fileHash;
        posterPath = QString("%1%2%3%2%4.jpg").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
        qDebug() << "Trying to save poster to" << posterPath;
        show->posterImage()->save(posterPath, "jpg", 100);
        qDebug() << "Trying to save poster to" << show->dir() + QDir::separator() + "poster.jpg";
        show->posterImage()->save(show->dir() + QDir::separator() + "poster.jpg", "jpg", 100);

        // English
        fileHash = hash(QString("season%1* All Seasons").arg(path));
        qDebug() << "(english) fileHash=" << fileHash;
        posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
        qDebug() << "Trying to save poster to" << posterPath;
        show->posterImage()->save(posterPath, "jpg", 100);

        // German
        fileHash = hash(QString("season%1* Alle Staffeln").arg(path));
        qDebug() << "(german) fileHash=" << fileHash;
        posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
        qDebug() << "Trying to save poster to" << posterPath;
        show->posterImage()->save(posterPath, "jpg", 100);
    }

    // backdrop
    if (show->backdropImageChanged() && !show->backdropImage()->isNull()) {
        qDebug() << "Backdrop has changed";
        QString fanartHash = hash(path);
        qDebug() << "Hash for" << path << "is" << fanartHash;
        QString fanartPath = QString("%1%2Video%2Fanart%2%3.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash);
        qDebug() << "Trying to save backdrop to" << fanartPath;
        show->backdropImage()->save(fanartPath, "jpg", 100);
    }

    // banner
    if (show->bannerImageChanged() && !show->bannerImage()->isNull()) {
        qDebug() << "Banner has changed";
        QString bannerHash = hash(path);
        qDebug() << "Hash for" << path << "is" << bannerHash;
        QString bannerPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(bannerHash.left(1)).arg(bannerHash);
        qDebug() << "Trying to save banner to" << bannerPath;
        show->bannerImage()->save(bannerPath, "jpg", 100);
    }

    // season images
    foreach (int season, show->seasons()) {
        if (!show->seasonPosterImageChanged(season) || show->seasonPosterImage(season)->isNull())
            continue;

        // English
        QString seasonHash = hash(QString("season%1Season %2").arg(path).arg(season));
        QString seasonPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(seasonHash.left(1)).arg(seasonHash);
        show->seasonPosterImage(season)->save(seasonPath, "jpg", 100);

        // German
        seasonHash = hash(QString("season%1Staffel %2").arg(path).arg(season));
        seasonPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(seasonHash.left(1)).arg(seasonHash);
        show->seasonPosterImage(season)->save(seasonPath, "jpg", 100);
    }

    // actors
    foreach (Actor actor, show->actors()) {
        if (actor.image.isNull())
            continue;
        QString hashActor = actorHash(actor);
        QString actorThumb = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hashActor.left(1)).arg(hashActor);
        actor.image.save(actorThumb, "jpg", 100);
    }

    return true;
}

/**
 * @brief Saves a tv show episode (including info, poster, backdrops)
 * @param episode Episode to save
 * @return Saving success
 */
bool XbmcSql::saveTvShowEpisode(TvShowEpisode *episode)
{
    qDebug() << "Entered, episode=" << episode->name();
    if (!episode->tvShow()) {
        qWarning() << "Episode has no show";
        return false;
    }

    if (episode->files().isEmpty()) {
        qWarning() << "Episode has no files";
        return false;
    }

    QSqlQuery query(db());
    int idShow = -1;

    QString path = tvShowMediaCenterPath(episode->tvShow()->dir());
    qDebug() << "Dir" << episode->tvShow()->dir() << "becomes" << path;
    if (path.contains("\\") && !path.endsWith("\\"))
        path.append("\\");
    else if (path.contains("/") && !path.endsWith("/"))
        path.append("/");
    qDebug() << "Path is now" << path;
    // get show id
    query.prepare("SELECT idShow FROM tvshow WHERE c16=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idShow = query.value(query.record().indexOf("idShow")).toInt();
        qDebug() << "Found show, idShow=" << idShow;
    }

    // save the show first
    if (idShow == -1) {
        qDebug() << "Show not found, saving show first";
        episode->tvShow()->saveData(this);
    }

    // check again
    query.prepare("SELECT idShow FROM tvshow WHERE c16=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idShow = query.value(query.record().indexOf("idShow")).toInt();
        qDebug() << "Got show, idShow=" << idShow;
    }

    if (idShow == -1) {
        qDebug() << "No show found, giving up";
        return false;
    }

    // get Path ID
    QString episodePath = tvShowMediaCenterPath(episode->files().at(0));
    qDebug() << "First file" << episode->files().at(0) << "becomes episodePath=" << episodePath;
    QStringList episodePathSplit;
    QString joiner = (episodePath.contains("/")) ? "/" : "\\";
    episodePathSplit = episodePath.split(joiner);
    if (episodePathSplit.count() > 0 )
        episodePathSplit.takeLast();
    episodePath = episodePathSplit.join(joiner);

    if (episodePath.contains("\\") && !episodePath.endsWith("\\"))
        episodePath.append("\\");
    else if (episodePath.contains("/") && !episodePath.endsWith("/"))
        episodePath.append("/");

    qDebug() << "episodePath=" << episodePath;

    int idPath = -1;
    query.prepare("SELECT idPath FROM path WHERE strPath=:path");
    query.bindValue(":path", episodePath);
    query.exec();
    if (query.next()) {
        idPath = query.value(query.record().indexOf("idPath")).toInt();
        qDebug() << "Found path, idPath=" << idPath;
    } else {
        query.prepare("INSERT INTO path(strPath, strContent, strScraper, scanRecursive, useFolderNames, noUpdate, exclude) "
                      "VALUES(:path, 'tvshows', 'metadata.tvdb.com', 0, 1, 1, 0)");
        query.bindValue(":path", episodePath);
        query.exec();
        idPath = query.lastInsertId().toInt();
        qDebug() << "Inserting new path, idPath=" << idPath;
    }

    // get file id
    int idFile = -1;
    int idEpisode = -1;
    QString sqlWhereFile;
    if (episode->files().count() == 1) {
        qDebug() << "Single file episode";
        QFileInfo fi(episode->files().at(0));
        sqlWhereFile = QString("strFilename='%1' AND idPath='%2'").arg(fi.fileName().replace("'", "''")).arg(idPath);
    } else {
        qDebug() << "Multifile episode";
        QString file = tvShowMediaCenterPath(episode->files().at(0));
        file.replace("'", "''");
        sqlWhereFile = QString("strFilename LIKE 'stack://%%1%'").arg(file);
    }
    qDebug() << "sqlWhereFile=" << sqlWhereFile;
    query.prepare("SELECT idFile, strFilename FROM files WHERE " + sqlWhereFile);
    query.exec();
    if (episode->files().count() == 1) {
        if (query.next()) {
            idFile = query.value(query.record().indexOf("idFile")).toInt();
            qDebug() << "Found file" << idFile;
        }
    } else {
        while (idFile == -1 && query.next()) {
            QString path = query.value(query.record().indexOf("strFilename")).toString();
            path.replace("stack://", "");
            QStringList dbPaths = path.split(" , ", QString::SkipEmptyParts);
            QStringList filePaths;
            foreach (const QString &path, episode->files())
                filePaths << mediaCenterPath(path);
            qSort(filePaths);
            qSort(dbPaths);
            if (dbPaths == filePaths)
                idFile = query.value(query.record().indexOf("idFile")).toInt();
        }
    }
    qDebug() << "idFile=" << idFile;

    // update file data and get episode id, or insert file
    if (idFile != -1) {
        query.prepare("SELECT idEpisode FROM episode WHERE idFile=:idFile");
        query.bindValue(":idFile", idFile);
        query.exec();
        if (query.next()) {
            idEpisode = query.value(query.record().indexOf("idEpisode")).toInt();
            qDebug() << "Found episode, idEpisode=" << idEpisode;
        }
        query.prepare("UPDATE files SET playCount=:playCount, lastPlayed=:lastPlayed WHERE idFile=:idFile");
        query.bindValue(":idFile", idFile);
        if (episode->playCount() == 0)
            query.bindValue(":playCount", QVariant(QVariant::Int));
        else
            query.bindValue(":playCount", episode->playCount());
        query.bindValue(":lastPlayed", episode->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
        query.exec();
    } else {
        QString filename;
        if (episode->files().count() == 1) {
            QFileInfo fiFile(episode->files().at(0));
            filename = fiFile.fileName();
        } else {
            QStringList files;
            foreach (const QString &file, episode->files())
                files << tvShowMediaCenterPath(file);
            filename = QString("stack://%1").arg(files.join(" , "));
        }
        query.prepare("INSERT INTO files(idPath, strFilename, playCount, lastPlayed) VALUES(:idPath, :filename, :playCount, :lastPlayed)");
        query.bindValue(":idPath", idPath);
        query.bindValue(":filename", filename);
        if (episode->playCount() == 0)
            query.bindValue(":playCount", QVariant(QVariant::Int));
        else
            query.bindValue(":playCount", episode->playCount());
        query.bindValue(":lastPlayed", episode->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
        query.exec();
        idFile = query.lastInsertId().toInt();
        qDebug() << "Inserted new file, idFile=" << idFile;
    }

    // create xml data for thumbnail
    QByteArray thumbnails;
    QXmlStreamWriter xml(&thumbnails);
    if (!episode->thumbnail().isEmpty()) {
        xml.writeStartElement("thumb");
        xml.writeCharacters(episode->thumbnail().toString());
        xml.writeEndElement();
    }

    if (idEpisode != -1) {
        qDebug() << "Updating episode";
        query.prepare("UPDATE episode SET "
                      "c00=:title, "
                      "c01=:plot, "
                      "c03=:rating, "
                      "c04=:writer, "
                      "c05=:firstAired, "
                      "c06=:thumbnails, "
                      "c10=:director, "
                      "c12=:season, "
                      "c13=:episode, "
                      "c15='-1', "
                      "c16='-1' "
                      "WHERE idEpisode=:idEpisode");
        query.bindValue(":title", episode->name());
        query.bindValue(":plot", episode->overview());
        query.bindValue(":rating", episode->rating());
        query.bindValue(":writer", episode->writers().join(" / "));
        query.bindValue(":firstAired", episode->firstAired().toString("yyyy-MM-dd"));
        query.bindValue(":thumbnails", QString(thumbnails));
        query.bindValue(":director", episode->directors().join(" / "));
        query.bindValue(":season", episode->season());
        query.bindValue(":episode", episode->episode());
        query.bindValue(":idEpisode", idEpisode);
        query.exec();
    } else {
        QString file;
        if (episode->files().count() == 1) {
            file = tvShowMediaCenterPath(episode->files().at(0));
        } else {
            QStringList files;
            foreach (const QString &mFile, episode->files())
                files << tvShowMediaCenterPath(mFile);
            file = "stack://" + files.join(" , ");
        }
        qDebug() << "Inserting new episode with file=" << file;
        query.prepare("INSERT INTO episode(idFile, c00, c01, c03, c04, c05, c06, c10, c12, c13, c15, c16, c18, c19) "
                      "VALUES(:idFile, :title, :plot, :rating, :writer, :firstAired, :thumbnails, :director, :season, :episode, '-1', '-1', :file, :idPath)");
        query.bindValue(":idFile", idFile);
        query.bindValue(":title", episode->name());
        query.bindValue(":plot", episode->overview());
        query.bindValue(":rating", episode->rating());
        query.bindValue(":writer", episode->writers().join(" / "));
        query.bindValue(":firstAired", episode->firstAired().toString("yyyy-MM-dd"));
        query.bindValue(":thumbnails", QString(thumbnails));
        query.bindValue(":director", episode->directors().join(" / "));
        query.bindValue(":season", episode->season());
        query.bindValue(":episode", episode->episode());
        query.bindValue(":idPath", idPath);
        query.bindValue(":file", file);
        query.exec();
        idEpisode = query.lastInsertId().toInt();
        qDebug() << "idEpisode=" << idEpisode;
    }

    // Link TvShow -> Episode
    query.prepare("SELECT COUNT(*) FROM tvshowlinkepisode WHERE idShow=:idShow AND idEpisode=:idEpisode");
    query.bindValue(":idShow", idShow);
    query.bindValue(":idEpisode", idEpisode);
    query.exec();
    query.next();
    if (query.value(0).toInt() == 0) {
        query.prepare("INSERT INTO tvshowlinkepisode(idShow, idEpisode) VALUES(:idShow, :idEpisode)");
        query.bindValue(":idShow", idShow);
        query.bindValue(":idEpisode", idEpisode);
        query.exec();
    }

    // Writers
    query.prepare("DELETE FROM writerlinkepisode WHERE idEpisode=:idEpisode");
    query.bindValue(":idEpisode", idEpisode);
    query.exec();
    foreach (const QString &writer, episode->writers()) {
        int idActor = -1;
        query.prepare("SELECT idActor FROM actors WHERE strActor=:actor");
        query.bindValue(":actor", writer);
        query.exec();
        if (query.next()) {
            idActor = query.value(query.record().indexOf("idActor")).toInt();
        } else {
            query.prepare("INSERT INTO actors(strActor) VALUES(:actor)");
            query.bindValue(":actor", writer);
            query.exec();
            idActor = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO writerlinkepisode(idWriter, idEpisode) VALUES(:idWriter, :idEpisode)");
        query.bindValue(":idWriter", idActor);
        query.bindValue(":idEpisode", idEpisode);
        query.exec();
    }

    // Directors
    query.prepare("DELETE FROM directorlinkepisode WHERE idEpisode=:idEpisode");
    query.bindValue(":idEpisode", idEpisode);
    query.exec();
    foreach (const QString &director, episode->directors()) {
        int idActor = -1;
        query.prepare("SELECT idActor FROM actors WHERE strActor=:actor");
        query.bindValue(":actor", director);
        query.exec();
        if (query.next()) {
            idActor = query.value(query.record().indexOf("idActor")).toInt();
        } else {
            query.prepare("INSERT INTO actors(strActor) VALUES(:actor)");
            query.bindValue(":actor", director);
            query.exec();
            idActor = query.lastInsertId().toInt();
        }
        query.prepare("INSERT INTO directorlinkepisode(idDirector, idEpisode) VALUES(:idDirector, :idEpisode)");
        query.bindValue(":idDirector", idActor);
        query.bindValue(":idEpisode", idEpisode);
        query.exec();
    }

    // save images
    if (episode->thumbnailImageChanged() && !episode->thumbnailImage()->isNull()) {
        qDebug() << "Thumbnail image has changed";
        QString thumbHash = hash(tvShowMediaCenterPath(episode->files().at(0)));
        qDebug() << "First file" << episode->files().at(0) << "becomes" << tvShowMediaCenterPath(episode->files().at(0)) << "with hash" << thumbHash;
        QString thumbPath = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(thumbHash.left(1)).arg(thumbHash);
        qDebug() << "Trying to save thumbnail to" << thumbPath;
        episode->thumbnailImage()->save(thumbPath, "jpg", 100);
    }

    return true;
}

/**
 * @brief Loads a poster for a movie set
 * @param setName The name of the set
 * @return The poster image
 */
QImage XbmcSql::movieSetPoster(QString setName)
{
    qDebug() << "Entered, setName=" << setName;
    QString hash = movieSetHash(setName);
    QString path = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hash.left(1)).arg(hash);
    qDebug() << "Trying to load from" << path;
    QFileInfo fi(path);
    if (!fi.isFile()) {
        qDebug() << "File doesn't exist";
        return QImage();
    }
    QImage img(path);
    return img;
}

/**
 * @brief Loads a backdrop for a movie set
 * @param setName The name of the set
 * @return The backdrop image
 */
QImage XbmcSql::movieSetBackdrop(QString setName)
{
    qDebug() << "Entered, setName=" << setName;
    QString hash = movieSetHash(setName);
    QString path = QString("%1%2Video%2Fanart%2%3.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hash);
    qDebug() << "Trying to load from" << path;
    QFileInfo fi(path);
    if (!fi.isFile()) {
        qDebug() << "File doesn't exist";
        return QImage();
    }
    QImage img(path);
    return img;
}

/**
 * @brief Saves a movie set poster
 * @param setName The name of the set
 * @param poster The poster to save
 */
void XbmcSql::saveMovieSetPoster(QString setName, QImage poster)
{
    qDebug() << "Entered, setName=" << setName;
    QString hash = movieSetHash(setName);
    QString path = QString("%1%2Video%2%3%2%4.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hash.left(1)).arg(hash);
    qDebug() << "Trying to save to" << path;
    poster.save(path, "jpg", 100);
}

/**
 * @brief Saves a movie set backdrpo
 * @param setName The name of the set
 * @param backdrop The backdrop image
 */
void XbmcSql::saveMovieSetBackdrop(QString setName, QImage backdrop)
{
    qDebug() << "Entered, setName=" << setName;
    QString hash = movieSetHash(setName);
    QString path = QString("%1%2Video%2Fanart%2%3.tbn").arg(Settings::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hash);
    qDebug() << "Trying to save backdrop to" << path;
    backdrop.save(path, "jpg", 100);
}

/**
 * @brief Computes the thumbnail hash for a string
 * @param string String
 * @return Hash
 */
QString XbmcSql::hash(QString string)
{
    qDebug() << "Entered, string=" << string;
    QString chars = string.toLower();
    uint crc = 0xffffffff;
    QByteArray bytes = chars.toUtf8();
    foreach (char byte, bytes) {
        crc ^= ((uint)(byte) << 24);
        for (int i=0 ; i<8; ++i) {
            if ((crc & 0x80000000) == 0x80000000)
                crc = (crc << 1) ^ 0x04C11DB7;
            else
                crc <<= 1;
        }
    }

    QString number = QString::number(crc, 16);
    QString hash = number.rightJustified(8, QChar('0'));
    qDebug() << "hash=" << hash;
    return hash;
}

/**
 * @brief Computes the thumbnail hash for an actor
 * @param actor Actor
 * @return Hash
 */
QString XbmcSql::actorHash(Actor actor)
{
    return hash(QString("actor%1").arg(actor.name));
}

/**
 * @brief Computes the thumbnail hash of a movie setname
 * @param setName Setname to get the hash for
 * @return Hash
 */
QString XbmcSql::movieSetHash(QString setName)
{
    qDebug() << "Entered, setName=" << setName;
    QSqlQuery query(db());
    query.prepare("SELECT idSet FROM sets WHERE strSet=:setName");
    query.bindValue(":setName", setName);
    query.exec();
    if (!query.next())
        return QString();
    int id = query.value(0).toInt();
    qDebug() << "id=" << id;

    return hash(QString("videodb://1/7/%1/").arg(id));
}

/**
 * @brief DB Object getter
 * @return Database object
 */
QSqlDatabase XbmcSql::db()
{
    return *m_db;
}

/**
 * @brief Replaces the path with the nearest MediaCenterPath for movies given in the settings
 * It also adjusts the directory separator
 * @param file Filename
 * @return File with replaced path
 */
QString XbmcSql::mediaCenterPath(QString file)
{
    qDebug() << "Entered, file=" << file;
    QList<SettingsDir> dirs = Settings::instance()->movieDirectories();
    SettingsDir dir;
    file = QDir::toNativeSeparators(file);
    QString mediaCenterFile = file;
    for (int i=0, n=dirs.count() ; i<n ; ++i) {
        if (file.startsWith(dirs.at(i).path) && dirs.at(i).path.length() > dir.path.length())
            dir = dirs.at(i);
    }
    if (!dir.mediaCenterPath.isEmpty()) {
        mediaCenterFile.replace(dir.path, dir.mediaCenterPath);
        if (dir.mediaCenterPath.contains("\\"))
            mediaCenterFile.replace("/", "\\");
        else
            mediaCenterFile.replace("\\", "/");
    }

    qDebug() << "mediaCenterFile=" << mediaCenterFile;

    return mediaCenterFile;
}

/**
 * @brief Returns the nearest MediaCenterPath given in the settings for a movie file
 * If the path is empty this function returns the path
 * @param file Complete filename
 * @return MediaCenterPath
 */
QString XbmcSql::mediaCenterDir(QString file)
{
    qDebug() << "Entered, file=" << file;
    QList<SettingsDir> dirs = Settings::instance()->movieDirectories();
    SettingsDir dir;
    for (int i=0, n=dirs.count() ; i<n ; ++i) {
        if (file.startsWith(dirs.at(i).path) && dirs.at(i).path.length() > dir.path.length())
            dir = dirs.at(i);
    }

    qDebug() << "path=" << dir.path << "mediaCenterPath=" << dir.mediaCenterPath;

    if (dir.mediaCenterPath.isEmpty())
        return dir.path;
    else
        return dir.mediaCenterPath;
}

/**
 * @brief Replaces the path with the nearest MediaCenterPath for tv shows given in the settings
 * It also adjusts the directory separator
 * @param file Filename
 * @return File with replaced path
 */
QString XbmcSql::tvShowMediaCenterPath(QString file)
{
    qDebug() << "Entered, file=" << file;
    QList<SettingsDir> dirs = Settings::instance()->tvShowDirectories();
    SettingsDir dir;
    file = QDir::toNativeSeparators(file);
    QString mediaCenterFile = file;
    for (int i=0, n=dirs.count() ; i<n ; ++i) {
        if (file.startsWith(dirs.at(i).path) && dirs.at(i).path.length() > dir.path.length())
            dir = dirs.at(i);
    }
    if (!dir.mediaCenterPath.isEmpty()) {
        mediaCenterFile.replace(dir.path, dir.mediaCenterPath);
        if (dir.mediaCenterPath.contains("\\"))
            mediaCenterFile.replace("/", "\\");
        else
            mediaCenterFile.replace("\\", "/");
    }

    qDebug() << "mediaCenterFile=" << mediaCenterFile;

    return mediaCenterFile;
}

/**
 * @brief Returns the nearest MediaCenterPath given in the settings for a tv show file
 * If the path is empty this function returns the path
 * @param file Complete filename
 * @return MediaCenterPath
 */
QString XbmcSql::tvShowMediaCenterDir(QString file)
{
    qDebug() << "Entered, file=" << file;
    QList<SettingsDir> dirs = Settings::instance()->tvShowDirectories();
    SettingsDir dir;
    for (int i=0, n=dirs.count() ; i<n ; ++i) {
        if (file.startsWith(dirs.at(i).path) && dirs.at(i).path.length() > dir.path.length())
            dir = dirs.at(i);
    }

    qDebug() << "path=" << dir.path << "mediaCenterPath=" << dir.mediaCenterPath;

    if (dir.mediaCenterPath.isEmpty())
        return dir.path;
    else
        return dir.mediaCenterPath;
}

/**
 * @brief Replaces the path with the nearest MediaCenterPath for concerts given in the settings
 * It also adjusts the directory separator
 * @param file Filename
 * @return File with replaced path
 */
QString XbmcSql::concertMediaCenterPath(QString file)
{
    qDebug() << "Entered, file=" << file;
    QList<SettingsDir> dirs = Settings::instance()->concertDirectories();
    SettingsDir dir;
    file = QDir::toNativeSeparators(file);
    QString mediaCenterFile = file;
    for (int i=0, n=dirs.count() ; i<n ; ++i) {
        if (file.startsWith(dirs.at(i).path) && dirs.at(i).path.length() > dir.path.length())
            dir = dirs.at(i);
    }
    if (!dir.mediaCenterPath.isEmpty()) {
        mediaCenterFile.replace(dir.path, dir.mediaCenterPath);
        if (dir.mediaCenterPath.contains("\\"))
            mediaCenterFile.replace("/", "\\");
        else
            mediaCenterFile.replace("\\", "/");
    }

    qDebug() << "mediaCenterFile=" << mediaCenterFile;

    return mediaCenterFile;
}

/**
 * @brief Returns the nearest MediaCenterPath given in the settings for a concert file
 * If the path is empty this function returns the path
 * @param file Complete filename
 * @return MediaCenterPath
 */
QString XbmcSql::concertMediaCenterDir(QString file)
{
    qDebug() << "Entered, file=" << file;
    QList<SettingsDir> dirs = Settings::instance()->concertDirectories();
    SettingsDir dir;
    for (int i=0, n=dirs.count() ; i<n ; ++i) {
        if (file.startsWith(dirs.at(i).path) && dirs.at(i).path.length() > dir.path.length())
            dir = dirs.at(i);
    }

    qDebug() << "path=" << dir.path << "mediaCenterPath=" << dir.mediaCenterPath;

    if (dir.mediaCenterPath.isEmpty())
        return dir.path;
    else
        return dir.mediaCenterPath;
}
