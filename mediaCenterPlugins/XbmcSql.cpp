#include <QDir>
#include <QDomDocument>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QXmlStreamWriter>
#include <QtSql>

#include <QMessageBox>

#include "MessageBox.h"
#include "SettingsWidget.h"
#include "XbmcSql.h"

XbmcSql::XbmcSql(QObject *parent)
{
    setParent(parent);
    m_db = 0;
    m_isMySQL = false;
}

XbmcSql::~XbmcSql()
{
}

bool XbmcSql::hasFeature(int feature)
{
    if (feature == MediaCenterFeatures::EditTvShowEpisodeCertification)
        return false;
    if (feature == MediaCenterFeatures::EditTvShowEpisodeShowTitle)
        return false;
    if (feature == MediaCenterFeatures::EditTvShowEpisodeNetwork)
        return false;

    return true;
}

void XbmcSql::connectMysql(QString host, QString database, QString username, QString password)
{
    if (m_db) {
        m_db->close();
        delete m_db;
    }

    m_isMySQL = true;
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL", "xbmc"));
    m_db->setHostName(host);
    m_db->setDatabaseName(database);
    m_db->setUserName(username);
    m_db->setPassword(password);
    if (!m_db->open())
        MessageBox::instance()->showMessage(tr("Connection to XBMC MySQL Database failed!"));
}

void XbmcSql::connectSqlite(QString database)
{
    if (m_db) {
        m_db->close();
        delete m_db;
    }
    m_isMySQL = false;
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "xbmc"));
    m_db->setDatabaseName(database);
    if (!m_db->open())
        MessageBox::instance()->showMessage(tr("Connection to XBMC SQLite Database failed!"));
}

void XbmcSql::shutdown()
{
    if (m_db) {
        m_db->close();
        QString connection = m_db->connectionName();
        delete m_db;
        QSqlDatabase::removeDatabase(connection);
    }
}

bool XbmcSql::saveMovie(Movie *movie)
{
    QSqlQuery query(db());

    // get Path ID
    int idPath = -1;
    QFileInfo fiPath(mediaCenterPath(movie->files().at(0)));
    QString path = fiPath.path();
    if (mediaCenterPath(movie->files().at(0)).contains("\\")) {
        path.replace("/", "\\");
        if (!path.endsWith("\\"))
            path.append("\\");
    } else {
        path.replace("\\", "/");
        if (!path.endsWith("/"))
            path.append("/");
    }

    query.prepare("SELECT idPath FROM path WHERE strPath=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idPath = query.value(query.record().indexOf("idPath")).toInt();
    } else {
        query.prepare("INSERT INTO path(strPath, strContent, strScraper, scanRecursive, useFolderNames, noUpdate, exclude) "
                      "VALUES(:path, 'movies', 'metadata.themoviedb.org', 0, 0, 1, 0)");
        query.bindValue(":path", path);
        query.exec();
        idPath = query.lastInsertId().toInt();
    }

    // get File ID
    int idMovie = -1;
    int idFile = -1;
    if (movie->files().count() == 1) {
        query.prepare("SELECT idFile, strFilename FROM files WHERE strFilename=:fileName AND idPath=:idPath");
        query.bindValue(":idPath", idPath);
        query.bindValue(":fileName", fiPath.fileName());
    } else {
        QString fileName = fiPath.fileName();
        query.prepare(QString("SELECT idFile, strFilename FROM files WHERE strFilename LIKE 'stack://%%1%' AND idPath='%2'").arg(fileName).arg(idPath));
    }
    query.exec();
    if (movie->files().count() == 1) {
        if (query.next())
            idFile = query.value(query.record().indexOf("idFile")).toInt();
    } else {
        while (idFile == -1 && query.next()) {
            QString path = query.value(query.record().indexOf("strFilename")).toString();
            path.replace("stack://", "");
            QStringList dbPaths = path.split(" , ", QString::SkipEmptyParts);
            QStringList filePaths;
            foreach (const QString &path, movie->files())
                filePaths << mediaCenterPath(path);
            qSort(filePaths);
            qSort(dbPaths);
            if (dbPaths == filePaths)
                idFile = query.value(query.record().indexOf("idFile")).toInt();
        }
    }

    // update file data and get movie id, or insert file
    if (idFile != -1) {
        query.prepare("SELECT idMovie FROM movie WHERE idFile=:idFile");
        query.bindValue(":idFile", idFile);
        query.exec();
        if (query.next())
            idMovie = query.value(query.record().indexOf("idMovie")).toInt();
        query.prepare("UPDATE files SET playCount=:playCount, lastPlayed=:lastPlayed WHERE idFile=:idFile");
        query.bindValue(":idFile", idFile);
        if (movie->playcount() == 0)
            query.bindValue(":playCount", QVariant(QVariant::Int));
        else
            query.bindValue(":playCount", movie->playcount());
        query.bindValue(":lastPlayed", movie->lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
        query.exec();
    } else {
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
        idFile = query.lastInsertId().toInt();
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
        QString thumbnails;
        QString fanart;
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
        query.bindValue(":trailer", movie->trailer());
        query.bindValue(":fanart", QString(fanart));
        query.bindValue(":countries", movie->countries().join(" / "));
        query.exec();
    } else {
        QString file;
        if (movie->files().count() == 1) {
            file = mediaCenterPath(movie->files().at(0));
            bool isUnixFile = !file.contains("\\");
            QStringList fileSplit = (isUnixFile) ? file.split("/", QString::SkipEmptyParts) : file.split("\\", QString::SkipEmptyParts);
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
            foreach (const QString &mFile, movie->files())
                files << mediaCenterPath(mFile);
            file = "stack://" + files.join(" , ");
        }

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
        query.bindValue(":trailer", movie->trailer());
        query.bindValue(":fanart", QString(fanart));
        query.bindValue(":countries", movie->countries().join(" / "));
        query.bindValue(":file", file);
        query.bindValue(":idPath", idPath);
        query.exec();
        idMovie = query.lastInsertId().toInt();
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
    QString fileHash = hash(mediaCenterPath(movie->files().at(0)));
    QString fanartHash = fileHash;
    if (movie->files().count() > 1) {
        QStringList files;
        foreach (const QString &file, movie->files())
            files << mediaCenterPath(file);
        fanartHash = hash(QString("stack://%1").arg(files.join(" , ")));
    }

    QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
    QString fanartPath = QString("%1%2Video%2Fanart%2%3.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash);
    if (movie->posterImageChanged() && !movie->posterImage()->isNull())
        movie->posterImage()->save(posterPath, "jpg", 100);
    if (movie->backdropImageChanged() && !movie->backdropImage()->isNull())
        movie->backdropImage()->save(fanartPath, "jpg", 100);

    foreach (Actor actor, movie->actors()) {
        if (actor.image.isNull())
            continue;
        QString hashActor = actorHash(actor);
        QString actorThumb = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hashActor.left(1)).arg(hashActor);
        actor.image.save(actorThumb, "jpg", 100);
    }

    return true;
}

bool XbmcSql::loadMovie(Movie *movie)
{
    if (movie->files().size() == 0)
        return false;
    QFileInfo fi(movie->files().at(0));
    if (!fi.isFile() ) {
        return false;
    }
    movie->clear();

    QString sqlWhereFile;
    QString file = mediaCenterPath(movie->files().at(0));
    if (movie->files().count() == 1) {
        bool isUnixFile = !file.contains("\\");
        QStringList fileSplit = (isUnixFile) ? file.split("/", QString::SkipEmptyParts) : file.split("\\", QString::SkipEmptyParts);
        if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0) ||
                                     QString::compare(fileSplit.last(), "index.bdmv", Qt::CaseInsensitive) == 0) {
            fileSplit.takeLast();
            if (!fileSplit.isEmpty() && (QString::compare(fileSplit.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0 ||
                                         QString::compare(fileSplit.last(), "BDMV", Qt::CaseInsensitive) == 0)) {
                fileSplit.takeLast();
            }
            file = (isUnixFile) ? fileSplit.join("/") : fileSplit.join("\\");
            file.prepend((isUnixFile) ? "/" : "\\");
            file.append((isUnixFile) ? "/" : "\\");
        }
        if (m_isMySQL)
            file.replace("\\", "\\\\");
        sqlWhereFile = QString("M.c22='%1'").arg(file);
    } else {
        if (m_isMySQL)
            file.replace("\\", "\\\\\\\\");
        sqlWhereFile = QString("M.c22 LIKE 'stack://%%1%'").arg(file);
    }

    QSqlQuery query(db());
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
                  "WHERE " + sqlWhereFile);
    query.exec();

    if (movie->files().count() == 1) {
        if (!query.next())
            return false;
    } else {
        bool found = false;
        while (!found && query.next()) {
            QString path = query.value(query.record().indexOf("filePath")).toString();
            path.replace("stack://", "");
            QStringList dbPaths = path.split(" , ", QString::SkipEmptyParts);
            QStringList filePaths;
            foreach (const QString &path, movie->files())
                filePaths << mediaCenterPath(path);
            qSort(filePaths);
            qSort(dbPaths);
            if (dbPaths == filePaths)
                found = true;
        }

        if (!found)
            return false;
    }

    QSqlRecord record = query.record();
    int idMovie = query.value(record.indexOf("idMovie")).toInt();
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

void XbmcSql::loadMovieImages(Movie *movie)
{
    if (movie->files().count() == 0)
        return;

    QString fileHash = hash(mediaCenterPath(movie->files().at(0)));
    QString fanartHash = fileHash;
    if (movie->files().count() > 1) {
        QStringList files;
        foreach (const QString &file, movie->files())
            files << mediaCenterPath(file);
        fanartHash = hash(QString("stack://%1").arg(files.join(" , ")));
    }

    QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
    QString fanartPath = QString("%1%2Video%2Fanart%2%3.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash);
    QFileInfo posterFi(posterPath);
    QFileInfo fanartFi(fanartPath);
    if (posterFi.isFile())
        movie->posterImage()->load(posterPath);
    if (fanartFi.isFile())
        movie->backdropImage()->load(fanartPath);

    foreach (Actor *actor, movie->actorsPointer()) {
        if (actor->imageHasChanged)
            continue;
        QString hashActor = actorHash(*actor);
        QString actorThumb = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hashActor.left(1)).arg(hashActor);
        QFileInfo fi(actorThumb);
        if (fi.isFile())
            actor->image.load(actorThumb);
    }
}

void XbmcSql::exportDatabase(QList<Movie *> movies, QList<TvShow *> shows, QString exportPath, QString pathSearch, QString pathReplace)
{
    Q_UNUSED(movies);
    Q_UNUSED(shows);
    Q_UNUSED(exportPath);
    Q_UNUSED(pathSearch);
    Q_UNUSED(pathReplace);
}

bool XbmcSql::loadTvShow(TvShow *show)
{
    if (show->dir().isEmpty())
        return false;
    show->clear();

    QString path = tvShowMediaCenterPath(show->dir());
    if (path.contains("\\") && !path.endsWith("\\"))
        path.append("\\");
    else if (path.contains("/") && !path.endsWith("/"))
        path.append("/");

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
    if (!query.next())
        return false;

    QSqlRecord record = query.record();
    int idShow = query.value(record.indexOf("idShow")).toInt();
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

void XbmcSql::loadTvShowImages(TvShow *show)
{
    // English
    QString fileHash = hash(QString("season%1* All Seasons").arg(show->mediaCenterPath()));
    QFileInfo posterFi(QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash));
    if (posterFi.isFile()) {
        show->posterImage()->load(posterFi.absoluteFilePath());
    } else {
        // German
        fileHash = hash(QString("season%1* Alle Staffeln").arg(show->mediaCenterPath()));
        posterFi.setFile(QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash));
        if (posterFi.isFile())
            show->posterImage()->load(posterFi.absoluteFilePath());
    }

    QString fanartHash = hash(show->mediaCenterPath());
    QFileInfo backdropFi(QString("%1%2Video%2Fanart%2%3.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash));
    if (backdropFi.isFile())
        show->backdropImage()->load(backdropFi.absoluteFilePath());

    QString bannerHash = hash(show->mediaCenterPath());
    QFileInfo bannerFi(QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(bannerHash.left(1)).arg(bannerHash));
    if (bannerFi.isFile())
        show->bannerImage()->load(bannerFi.absoluteFilePath());

    foreach (int season, show->seasons()) {
        // English
        QString seasonHash = hash(QString("season%1Season %2").arg(show->mediaCenterPath()).arg(season));
        QFileInfo seasonFi(QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(seasonHash.left(1)).arg(seasonHash));
        if (seasonFi.isFile()) {
            show->seasonPosterImage(season)->load(seasonFi.absoluteFilePath());
        } else {
            // German
            seasonHash = hash(QString("season%1Staffel %2").arg(show->mediaCenterPath()).arg(season));
            seasonFi.setFile(QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(seasonHash.left(1)).arg(seasonHash));
            if (seasonFi.isFile())
                show->seasonPosterImage(season)->load(seasonFi.absoluteFilePath());
        }
    }

    foreach (Actor *actor, show->actorsPointer()) {
        if (actor->imageHasChanged)
            continue;
        QString hashActor = actorHash(*actor);
        QString actorThumb = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hashActor.left(1)).arg(hashActor);
        QFileInfo fi(actorThumb);
        if (fi.isFile())
            actor->image.load(actorThumb);
    }
}

bool XbmcSql::loadTvShowEpisode(TvShowEpisode *episode)
{
    if (episode->files().count() == 0)
        return false;
    episode->clear();

    QString sqlWhereFile;
    if (episode->files().count() == 1)
        sqlWhereFile = QString("E.c18='%1'").arg(tvShowMediaCenterPath(episode->files().at(0)).replace("\\", "\\\\"));
    else
        sqlWhereFile = QString("E.c18 LIKE 'stack://%%1%'").arg(tvShowMediaCenterPath(episode->files().at(0)).replace("\\", "\\\\\\\\"));

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
        if (!query.next())
            return false;
    } else {
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
            if (dbPaths == filePaths)
                found = true;
        }

        if (!found)
            return false;
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

void XbmcSql::loadTvShowEpisodeImages(TvShowEpisode *episode)
{
    if (episode->files().count() == 0)
        return;

    QString fileHash = hash(tvShowMediaCenterPath(episode->files().at(0)));
    QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
    QFileInfo posterFi(posterPath);
    if (posterFi.isFile())
        episode->thumbnailImage()->load(posterPath);
}

bool XbmcSql::saveTvShow(TvShow *show)
{
    if (show->dir().isEmpty())
        return false;

    QSqlQuery query(db());

    // get Path ID
    int idPath = -1;
    QString path = show->mediaCenterPath();
    if (path.isEmpty()) {
        path = tvShowMediaCenterPath(show->dir());
        if (path.contains("\\") && !path.endsWith("\\"))
            path.append("\\");
        else if (path.contains("/") && !path.endsWith("/"))
            path.append("/");
    }
    query.prepare("SELECT idPath FROM path WHERE strPath=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idPath = query.value(query.record().indexOf("idPath")).toInt();
    } else {
        query.prepare("INSERT INTO path(strPath, strContent, strScraper, scanRecursive, useFolderNames, noUpdate, exclude) "
                      "VALUES(:path, 'tvshows', 'metadata.tvdb.com', 0, 1, 1, 0)");
        query.bindValue(":path", path);
        query.exec();
        idPath = query.lastInsertId().toInt();
    }

    int idShow = -1;
    query.prepare("SELECT idShow FROM tvshow WHERE c16=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next())
        idShow = query.value(query.record().indexOf("idShow")).toInt();

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
        QString fileHash = hash(path);
        QString posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
        show->posterImage()->save(posterPath, "jpg", 100);

        // poster.jpg
        fileHash = hash(path + "poster.jpg");
        posterPath = QString("%1%2%3%2%4.jpg").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
        show->posterImage()->save(posterPath, "jpg", 100);

        // English
        fileHash = hash(QString("season%1* All Seasons").arg(path));
        posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
        show->posterImage()->save(posterPath, "jpg", 100);

        // German
        fileHash = hash(QString("season%1* Alle Staffeln").arg(path));
        posterPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fileHash.left(1)).arg(fileHash);
        show->posterImage()->save(posterPath, "jpg", 100);
    }

    // backdrop
    if (show->backdropImageChanged() && !show->backdropImage()->isNull()) {
        QString fanartHash = hash(path);
        QString fanartPath = QString("%1%2Video%2Fanart%2%3.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(fanartHash);
        show->backdropImage()->save(fanartPath, "jpg", 100);
    }

    // banner
    if (show->bannerImageChanged() && !show->bannerImage()->isNull()) {
        QString bannerHash = hash(path);
        QString bannerPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(bannerHash.left(1)).arg(bannerHash);
        show->bannerImage()->save(bannerPath, "jpg", 100);
    }

    // season images
    foreach (int season, show->seasons()) {
        if (!show->seasonPosterImageChanged(season) || show->seasonPosterImage(season)->isNull())
            continue;

        // English
        QString seasonHash = hash(QString("season%1Season %2").arg(path).arg(season));
        QString seasonPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(seasonHash.left(1)).arg(seasonHash);
        show->seasonPosterImage(season)->save(seasonPath, "jpg", 100);

        // German
        seasonHash = hash(QString("season%1Staffel %2").arg(path).arg(season));
        seasonPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(seasonHash.left(1)).arg(seasonHash);
        show->seasonPosterImage(season)->save(seasonPath, "jpg", 100);
    }

    // actors
    foreach (Actor actor, show->actors()) {
        if (actor.image.isNull())
            continue;
        QString hashActor = actorHash(actor);
        QString actorThumb = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hashActor.left(1)).arg(hashActor);
        actor.image.save(actorThumb, "jpg", 100);
    }

    return true;
}

bool XbmcSql::saveTvShowEpisode(TvShowEpisode *episode)
{
    if (!episode->tvShow())
        return false;

    if (episode->files().isEmpty())
        return false;

    QSqlQuery query(db());
    int idShow = -1;

    QString path = tvShowMediaCenterPath(episode->tvShow()->dir());
    if (path.contains("\\") && !path.endsWith("\\"))
        path.append("\\");
    else if (path.contains("/") && !path.endsWith("/"))
        path.append("/");
    // get show id
    query.prepare("SELECT idShow FROM tvshow WHERE c16=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next())
        idShow = query.value(query.record().indexOf("idShow")).toInt();

    // save the show first
    if (idShow == -1)
        episode->tvShow()->saveData(this);

    // check again
    query.prepare("SELECT idShow FROM tvshow WHERE c16=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next())
        idShow = query.value(query.record().indexOf("idShow")).toInt();

    if (idShow == -1)
        return false;

    // get Path ID
    int idPath = -1;
    query.prepare("SELECT idPath FROM path WHERE strPath=:path");
    query.bindValue(":path", path);
    query.exec();
    if (query.next()) {
        idPath = query.value(query.record().indexOf("idPath")).toInt();
    } else {
        query.prepare("INSERT INTO path(strPath, strContent, strScraper, scanRecursive, useFolderNames, noUpdate, exclude) "
                      "VALUES(:path, 'tvshows', 'metadata.tvdb.com', 0, 1, 1, 0)");
        query.bindValue(":path", path);
        query.exec();
        idPath = query.lastInsertId().toInt();
    }

    // get file id
    int idFile = -1;
    int idEpisode = -1;
    QString sqlWhereFile;
    if (episode->files().count() == 1) {
        QFileInfo fi(episode->files().at(0));
        sqlWhereFile = QString("strFilename='%1'").arg(fi.fileName());
    } else {
        sqlWhereFile = QString("strFilename LIKE 'stack://%%1%'").arg(tvShowMediaCenterPath(episode->files().at(0)));
    }
    query.prepare("SELECT idFile, strFilename FROM files WHERE " + sqlWhereFile);
    query.exec();
    if (episode->files().count() == 1) {
        if (query.next())
            idFile = query.value(query.record().indexOf("idFile")).toInt();
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

    // update file data and get episode id, or insert file
    if (idFile != -1) {
        query.prepare("SELECT idEpisode FROM episode WHERE idFile=:idFile");
        query.bindValue(":idFile", idFile);
        query.exec();
        if (query.next())
            idEpisode = query.value(query.record().indexOf("idEpisode")).toInt();
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
        QString thumbHash = hash(tvShowMediaCenterPath(episode->files().at(0)));
        QString thumbPath = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(thumbHash.left(1)).arg(thumbHash);
        episode->thumbnailImage()->save(thumbPath, "jpg", 100);
    }

    return true;
}

QImage XbmcSql::movieSetPoster(QString setName)
{
    QString hash = movieSetHash(setName);
    QString path = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hash.left(1)).arg(hash);
    QFileInfo fi(path);
    if (!fi.isFile())
        return QImage();
    QImage img(path);
    return img;
}

QImage XbmcSql::movieSetBackdrop(QString setName)
{
    QString hash = movieSetHash(setName);
    QString path = QString("%1%2Video%2Fanart%2%3.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hash);
    QFileInfo fi(path);
    if (!fi.isFile())
        return QImage();
    QImage img(path);
    return img;
}

void XbmcSql::saveMovieSetPoster(QString setName, QImage poster)
{
    QString hash = movieSetHash(setName);
    QString path = QString("%1%2Video%2%3%2%4.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hash.left(1)).arg(hash);
    poster.save(path, "jpg", 100);
}

void XbmcSql::saveMovieSetBackdrop(QString setName, QImage backdrop)
{
    QString hash = movieSetHash(setName);
    QString path = QString("%1%2Video%2Fanart%2%3.tbn").arg(SettingsWidget::instance()->xbmcThumbnailPath()).arg(QDir::separator()).arg(hash);
    backdrop.save(path, "jpg", 100);
}

QString XbmcSql::hash(QString string)
{
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
    return number.rightJustified(8, QChar('0'));
}

QString XbmcSql::actorHash(Actor actor)
{
    return hash(QString("actor%1").arg(actor.name));
}

QString XbmcSql::movieSetHash(QString setName)
{
    QSqlQuery query(db());
    query.prepare("SELECT idSet FROM sets WHERE strSet=:setName");
    query.bindValue(":setName", setName);
    query.exec();
    if (!query.next())
        return QString();
    int id = query.value(0).toInt();

    return hash(QString("videodb://1/7/%1/").arg(id));
}

QSqlDatabase XbmcSql::db()
{
    return *m_db;
}

QString XbmcSql::mediaCenterPath(QString file)
{
    QList<SettingsDir> dirs = SettingsWidget::instance()->movieDirectories();
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

    return mediaCenterFile;
}

QString XbmcSql::mediaCenterDir(QString file)
{
    QList<SettingsDir> dirs = SettingsWidget::instance()->movieDirectories();
    SettingsDir dir;
    for (int i=0, n=dirs.count() ; i<n ; ++i) {
        if (file.startsWith(dirs.at(i).path) && dirs.at(i).path.length() > dir.path.length())
            dir = dirs.at(i);
    }

    if (dir.mediaCenterPath.isEmpty())
        return dir.path;
    else
        return dir.mediaCenterPath;
}

QString XbmcSql::tvShowMediaCenterPath(QString file)
{
    QList<SettingsDir> dirs = SettingsWidget::instance()->tvShowDirectories();
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

    return mediaCenterFile;
}

QString XbmcSql::tvShowMediaCenterDir(QString file)
{
    QList<SettingsDir> dirs = SettingsWidget::instance()->tvShowDirectories();
    SettingsDir dir;
    for (int i=0, n=dirs.count() ; i<n ; ++i) {
        if (file.startsWith(dirs.at(i).path) && dirs.at(i).path.length() > dir.path.length())
            dir = dirs.at(i);
    }

    if (dir.mediaCenterPath.isEmpty())
        return dir.path;
    else
        return dir.mediaCenterPath;
}
