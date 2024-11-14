#include "database/MoviePersistence.h"

#include "data/movie/Movie.h"
#include "database/Database.h"
#include "globals/Manager.h"
#include "log/Log.h"

#include <QSqlQuery>
#include <QSqlRecord>

namespace mediaelch {

MoviePersistence::MoviePersistence(Database& db) : m_db(db)
{
}

QSqlDatabase MoviePersistence::db()
{
    return m_db.db();
}

void MoviePersistence::clearAllMovies()
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM movies");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='movies'");
    query.exec();
    query.prepare("DELETE FROM movieFiles");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='movieFiles'");
    query.exec();
    query.prepare("DELETE FROM movieSubtitles");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='movieSubtitles'");
    query.exec();
}

void MoviePersistence::clearMoviesInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM movieFiles WHERE idMovie IN (SELECT idMovie FROM movies WHERE path=:path)");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    query.prepare("DELETE FROM movieSubtitles WHERE idMovie IN (SELECT idMovie FROM movies WHERE path=:path)");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    query.prepare("DELETE FROM movies WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
}

void MoviePersistence::addMovie(Movie* movie, DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO movies(content, lastModified, inSeparateFolder, hasPoster, hasBackdrop, hasLogo, "
                  "hasClearArt, hasCdArt, hasBanner, hasThumb, hasExtraFanarts, discType, path) "
                  "VALUES(:content, :lastModified, :inSeparateFolder, :hasPoster, :hasBackdrop, :hasLogo, "
                  ":hasClearArt, :hasCdArt, :hasBanner, :hasThumb, :hasExtraFanarts, :discType, :path)");
    query.bindValue(":content", movie->nfoContent().isEmpty() ? "" : movie->nfoContent().toUtf8());
    query.bindValue(
        ":lastModified", movie->fileLastModified().isNull() ? QDateTime::currentDateTime() : movie->fileLastModified());
    query.bindValue(":inSeparateFolder", (movie->inSeparateFolder() ? 1 : 0));
    query.bindValue(":hasPoster", movie->hasImage(ImageType::MoviePoster) ? 1 : 0);
    query.bindValue(":hasBackdrop", movie->hasImage(ImageType::MovieBackdrop) ? 1 : 0);
    query.bindValue(":hasLogo", movie->hasImage(ImageType::MovieLogo) ? 1 : 0);
    query.bindValue(":hasClearArt", movie->hasImage(ImageType::MovieClearArt) ? 1 : 0);
    query.bindValue(":hasCdArt", movie->hasImage(ImageType::MovieCdArt) ? 1 : 0);
    query.bindValue(":hasBanner", movie->hasImage(ImageType::MovieBanner) ? 1 : 0);
    query.bindValue(":hasThumb", movie->hasImage(ImageType::MovieThumb) ? 1 : 0);
    query.bindValue(":hasExtraFanarts", movie->images().hasExtraFanarts() ? 1 : 0);
    query.bindValue(":discType", static_cast<int>(movie->discType()));
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    int insertId = query.lastInsertId().toInt();

    for (const mediaelch::FilePath& file : movie->files()) {
        query.prepare("INSERT INTO movieFiles(idMovie, file) VALUES(:idMovie, :file)");
        query.bindValue(":idMovie", insertId);
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }

    for (const Subtitle* subtitle : movie->subtitles()) {
        query.prepare("INSERT INTO movieSubtitles(idMovie, files, language, forced) VALUES(:idMovie, :files, "
                      ":language, :forced)");
        query.bindValue(":idMovie", insertId);
        query.bindValue(":files", subtitle->files().join("%§%"));
        query.bindValue(":language", subtitle->language().isEmpty() ? "" : subtitle->language());
        query.bindValue(":forced", subtitle->forced() ? 1 : 0);
        query.exec();
    }

    m_db.setLabel(movie->files(), movie->label());

    movie->setDatabaseId(insertId);
}

void MoviePersistence::update(Movie* movie)
{
    QSqlQuery query(db());
    query.prepare("UPDATE movies SET content=:content WHERE idMovie=:idMovie");
    query.bindValue(":content", movie->nfoContent().isEmpty() ? "" : movie->nfoContent());
    query.bindValue(":idMovie", movie->databaseId().toInt());
    query.exec();

    query.prepare("DELETE FROM movieFiles WHERE idMovie=:idMovie");
    query.bindValue(":idMovie", movie->databaseId().toInt());
    query.exec();
    for (const mediaelch::FilePath& file : movie->files()) {
        query.prepare("INSERT INTO movieFiles(idMovie, file) VALUES(:idMovie, :file)");
        query.bindValue(":idMovie", movie->databaseId().toInt());
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }

    query.prepare("DELETE FROM movieSubtitles WHERE idMovie=:idMovie");
    query.bindValue(":idMovie", movie->databaseId().toInt());
    query.exec();
    for (const Subtitle* subtitle : movie->subtitles()) {
        query.prepare("INSERT INTO movieSubtitles(idMovie, files, language, forced) VALUES(:idMovie, :files, "
                      ":language, :forced)");
        query.bindValue(":idMovie", movie->databaseId().toInt());
        query.bindValue(":files", subtitle->files().join("%§%"));
        query.bindValue(":language", subtitle->language().isEmpty() ? "" : subtitle->language());
        query.bindValue(":forced", subtitle->forced() ? 1 : 0);
        query.exec();
    }
}

QVector<Movie*> MoviePersistence::moviesInDirectory(DirectoryPath path, QObject* movieParent)
{
    m_db.db().transaction();
    QSqlQuery query(db());
    query.prepare("SELECT M.idMovie, M.content, M.lastModified, M.inSeparateFolder, M.hasPoster, M.hasBackdrop, "
                  "M.hasLogo, M.hasClearArt, "
                  "M.hasCdArt, M.hasBanner, M.hasThumb, M.hasExtraFanarts, M.discType, MF.file, L.color "
                  "FROM movies M "
                  "LEFT JOIN movieFiles MF ON MF.idMovie=M.idMovie "
                  "LEFT JOIN labels L ON MF.file=L.fileName "
                  "WHERE path=:path "
                  "ORDER BY M.idMovie, MF.file");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();

    QMap<int, Movie*> movies;
    while (query.next()) {
        Movie* movie = nullptr;
        if (movies.contains(query.value(query.record().indexOf("idMovie")).toInt())) {
            movie = movies.value(query.value(query.record().indexOf("idMovie")).toInt());
            if (movie == nullptr) {
                // This *must* not happen because we just inserted it.
                qCCritical(generic) << "[Database] Movie is undefined but should exist!";
                continue;
            }

        } else {
            ColorLabel label = static_cast<ColorLabel>(query.value(query.record().indexOf("color")).toInt());
            movie = new Movie(QStringList(), movieParent);
            movie->blockSignals(true);
            movie->setDatabaseId(query.value(query.record().indexOf("idMovie")).toInt());
            movie->setFileLastModified(query.value(query.record().indexOf("lastModified")).toDateTime());
            movie->setInSeparateFolder(query.value(query.record().indexOf("inSeparateFolder")).toInt() == 1);
            movie->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
            movie->images().setHasImage(
                ImageType::MoviePoster, query.value(query.record().indexOf("hasPoster")).toInt() == 1);
            movie->images().setHasImage(
                ImageType::MovieBackdrop, query.value(query.record().indexOf("hasBackdrop")).toInt() == 1);
            movie->images().setHasImage(
                ImageType::MovieLogo, query.value(query.record().indexOf("hasLogo")).toInt() == 1);
            movie->images().setHasImage(
                ImageType::MovieClearArt, query.value(query.record().indexOf("hasClearArt")).toInt() == 1);
            movie->images().setHasImage(
                ImageType::MovieCdArt, query.value(query.record().indexOf("hasCdArt")).toInt() == 1);
            movie->images().setHasImage(
                ImageType::MovieBanner, query.value(query.record().indexOf("hasBanner")).toInt() == 1);
            movie->images().setHasImage(
                ImageType::MovieThumb, query.value(query.record().indexOf("hasThumb")).toInt() == 1);
            movie->images().setHasExtraFanarts(query.value(query.record().indexOf("hasExtraFanarts")).toInt() == 1);
            movie->setDiscType(static_cast<DiscType>(query.value(query.record().indexOf("discType")).toInt()));
            movie->setLabel(label);
            movie->setChanged(false);
            movie->blockSignals(false);
            movies.insert(query.value(query.record().indexOf("idMovie")).toInt(), movie);
        }

        mediaelch::FileList files = movie->files();
        files << mediaelch::FilePath(query.value(query.record().indexOf("file")).toByteArray());
        movie->setFiles(files);
    }

    query.prepare("SELECT idMovie, files, language, forced FROM movieSubtitles");
    query.exec();
    while (query.next()) {
        int movieId = query.value(query.record().indexOf("idMovie")).toInt();
        Movie* movie = movies.value(movieId, nullptr);
        if (movie == nullptr) {
            continue;
        }
        auto* subtitle = new Subtitle(movie);
        subtitle->setForced(query.value(query.record().indexOf("forced")).toInt() == 1);
        subtitle->setLanguage(query.value(query.record().indexOf("language")).toString());
        subtitle->setFiles(query.value(query.record().indexOf("files")).toString().split("%§%"));
        subtitle->setChanged(false);
        movie->addSubtitle(subtitle, true);
    }

    m_db.db().commit();

    return movies.values().toVector();
}

} // namespace mediaelch
