#include "Database.h"

#include "concerts/Concert.h"
#include "data/Subtitle.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/Meta.h"
#include "media_centers/KodiXml.h"
#include "media_centers/kodi/EpisodeXmlWriter.h"
#include "movies/Movie.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "settings/Settings.h"
#include "tv_shows/TvShow.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

using namespace mediaelch;

Database::Database(QObject* parent) : QObject(parent)
{
    mediaelch::DirectoryPath dataLocation = Settings::instance()->databaseDir();
    QDir dir(dataLocation.dir());
    if (!dir.exists()) {
        dir.mkpath(dataLocation.toString());
    }
    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "mediaDb"));
    m_db->setDatabaseName(dataLocation.filePath("MediaElch.sqlite"));
    if (!m_db->open()) {
        qWarning() << "Could not open cache database";
    } else {
        QSqlQuery query(*m_db);

        int myDbVersion = -1;
        query.prepare("SELECT * FROM sqlite_master WHERE name ='settings' and type='table';");
        query.exec();
        if (query.next()) {
            query.prepare("SELECT value FROM settings WHERE idSettings=1");
            query.exec();
            if (query.next()) {
                myDbVersion = query.value(0).toInt();
            }
        }

        if (myDbVersion < 14) {
            query.prepare("DROP TABLE IF EXISTS movies;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS movieFiles;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS concerts;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS concertFiles;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS shows;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS showsSettings;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS episodes;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS showsEpisodes;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS episodeFiles;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS settings;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS importCache;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS labels;");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS movies ( "
                          "\"idMovie\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"content\" text NOT NULL, "
                          "\"lastModified\" integer NOT NULL, "
                          "\"inSeparateFolder\" integer NOT NULL, "
                          "\"hasPoster\" integer NOT NULL, "
                          "\"hasBackdrop\" integer NOT NULL, "
                          "\"hasLogo\" integer NOT NULL, "
                          "\"hasClearArt\" integer NOT NULL, "
                          "\"hasCdArt\" integer NOT NULL, "
                          "\"hasBanner\" integer NOT NULL, "
                          "\"hasThumb\" integer NOT NULL, "
                          "\"hasExtraFanarts\" integer NOT NULL, "
                          "\"discType\" integer NOT NULL, "
                          "\"path\" text NOT NULL);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS movieFiles( "
                          "\"idFile\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"idMovie\" integer NOT NULL, "
                          "\"file\" text NOT NULL "
                          ");");
            query.exec();
            query.prepare("CREATE INDEX id_movie_idx ON movieFiles(idMovie);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS concerts ( "
                          "\"idConcert\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"content\" text NOT NULL, "
                          "\"inSeparateFolder\" integer NOT NULL, "
                          "\"path\" text NOT NULL);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS concertFiles( "
                          "\"idFile\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"idConcert\" integer NOT NULL, "
                          "\"file\" text NOT NULL "
                          ");");
            query.exec();
            query.prepare("CREATE INDEX id_concert_idx ON concertFiles(idConcert);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS shows ( "
                          "\"idShow\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"dir\" text NOT NULL, "
                          "\"content\" text NOT NULL, "
                          "\"path\" text NOT NULL);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS showsSettings ( "
                          "\"idShow\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"tvdbid\" text NOT NULL, "
                          "\"url\" text NOT NULL, "
                          "\"showMissingEpisodes\" integer NOT NULL, "
                          "\"hideSpecialsInMissingEpisodes\" integer NOT NULL, "
                          "\"dir\" text NOT NULL);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS showsEpisodes ( "
                          "\"idEpisode\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"content\" text NOT NULL, "
                          "\"idShow\" integer NOT NULL, "
                          "\"seasonNumber\" integer NOT NULL, "
                          "\"episodeNumber\" integer NOT NULL, "
                          "\"tvdbid\" text NOT NULL, "
                          "\"updated\" integer NOT NULL);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS episodes ( "
                          "\"idEpisode\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"content\" text NOT NULL, "
                          "\"idShow\" integer NOT NULL, "
                          "\"seasonNumber\" integer NOT NULL, "
                          "\"episodeNumber\" integer NOT NULL, "
                          "\"path\" text NOT NULL);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS episodeFiles( "
                          "\"idFile\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"idEpisode\" integer NOT NULL, "
                          "\"file\" text NOT NULL "
                          ");");
            query.exec();
            query.prepare("CREATE INDEX id_episode_idx ON episodeFiles(idEpisode);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS labels ( "
                          "\"idLabel\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"color\" integer NOT NULL, "
                          "\"fileName\" text NOT NULL);");
            query.exec();
            query.prepare("CREATE INDEX id_label_filename_idx ON tags(fileName);");
            query.exec();


            query.prepare("CREATE TABLE IF NOT EXISTS importCache ( "
                          "\"id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"filename\" text NOT NULL, "
                          "\"type\" text NOT NULL, "
                          "\"path\" text NOT NULL);");
            query.exec();

            myDbVersion = 14;
            updateDbVersion(14);
        }

        if (myDbVersion < 15) {
            query.prepare("DROP TABLE IF EXISTS artists;");
            query.exec();
            query.prepare("DROP TABLE IF EXISTS albums;");
            query.exec();
            query.prepare("CREATE TABLE IF NOT EXISTS artists ( "
                          "\"idArtist\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"content\" text NOT NULL, "
                          "\"dir\" text NOT NULL, "
                          "\"path\" text NOT NULL);");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS albums ( "
                          "\"idAlbum\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"idArtist\" integer NOT NULL, "
                          "\"content\" text NOT NULL, "
                          "\"dir\" text NOT NULL, "
                          "\"path\" text NOT NULL);");
            query.exec();
            myDbVersion = 15;
            updateDbVersion(15);
        }

        if (myDbVersion < 16) {
            query.prepare("DROP TABLE IF EXISTS movieSubtitles;");
            query.exec();

            query.prepare("CREATE TABLE IF NOT EXISTS movieSubtitles( "
                          "\"idSubtitle\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                          "\"idMovie\" integer NOT NULL, "
                          "\"files\" text NOT NULL, "
                          "\"language\" text NOT NULL, "
                          "\"forced\" integer NOT NULL "
                          ");");
            query.exec();
            query.prepare("CREATE INDEX id_subtitle_idx ON movieSubtitles(idMovie);");
            query.exec();

            myDbVersion = 16;
            Q_UNUSED(myDbVersion);
            updateDbVersion(16);
        }

        query.prepare("PRAGMA synchronous=0;");
        query.exec();

        query.prepare("PRAGMA cache_size=20000;");
        query.exec();
    }
}

Database::~Database()
{
    if (m_db != nullptr && m_db->isOpen()) {
        m_db->close();
    }
    delete m_db;
    m_db = nullptr;
}

void Database::updateDbVersion(int version)
{
    QSqlQuery query(*m_db);
    query.prepare("SELECT * FROM sqlite_master WHERE name ='settings' and type='table';");
    query.exec();
    if (!query.next()) {
        query.prepare("CREATE TABLE IF NOT EXISTS settings( "
                      "\"idSettings\" integer NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "\"value\" text NOT NULL "
                      ");");
        query.exec();
    }

    query.prepare("SELECT value FROM settings WHERE idSettings=1");
    query.exec();
    if (query.next()) {
        query.prepare("UPDATE settings SET value=:dbVersion WHERE idSettings=1");
        query.bindValue(":dbVersion", QString::number(version));
        query.exec();
    } else {
        query.prepare("INSERT INTO settings(idSettings, value) VALUES(1, :dbVersion)");
        query.bindValue(":dbVersion", QString::number(version));
        query.exec();
    }
}

/**
 * \brief Returns an object to the cache database
 * \return Cache database object
 */
QSqlDatabase Database::db()
{
    return *m_db;
}

void Database::transaction()
{
    db().transaction();
}

void Database::commit()
{
    db().commit();
}

void Database::clearAllMovies()
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

void Database::clearMoviesInDirectory(DirectoryPath path)
{
    QMutexLocker locker(&m_mutex);

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

void Database::add(Movie* movie, DirectoryPath path)
{
    QMutexLocker locker(&m_mutex);

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

    setLabel(movie->files(), movie->label());

    movie->setDatabaseId(insertId);
}

void Database::update(Movie* movie)
{
    QSqlQuery query(db());
    query.prepare("UPDATE movies SET content=:content WHERE idMovie=:idMovie");
    query.bindValue(":content", movie->nfoContent().isEmpty() ? "" : movie->nfoContent());
    query.bindValue(":idMovie", movie->databaseId());
    query.exec();

    query.prepare("DELETE FROM movieFiles WHERE idMovie=:idMovie");
    query.bindValue(":idMovie", movie->databaseId());
    query.exec();
    for (const mediaelch::FilePath& file : movie->files()) {
        query.prepare("INSERT INTO movieFiles(idMovie, file) VALUES(:idMovie, :file)");
        query.bindValue(":idMovie", movie->databaseId());
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }

    query.prepare("DELETE FROM movieSubtitles WHERE idMovie=:idMovie");
    query.bindValue(":idMovie", movie->databaseId());
    query.exec();
    for (const Subtitle* subtitle : movie->subtitles()) {
        query.prepare("INSERT INTO movieSubtitles(idMovie, files, language, forced) VALUES(:idMovie, :files, "
                      ":language, :forced)");
        query.bindValue(":idMovie", movie->databaseId());
        query.bindValue(":files", subtitle->files().join("%§%"));
        query.bindValue(":language", subtitle->language().isEmpty() ? "" : subtitle->language());
        query.bindValue(":forced", subtitle->forced() ? 1 : 0);
        query.exec();
    }
}

QVector<Movie*> Database::moviesInDirectory(DirectoryPath path)
{
    QMutexLocker locker(&m_mutex);
    transaction();
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
                qCritical() << "[Database] Movie is undefined but should exist!";
                continue;
            }

        } else {
            ColorLabel label = static_cast<ColorLabel>(query.value(query.record().indexOf("color")).toInt());
            movie = new Movie(QStringList(), Manager::instance()->movieFileSearcher());
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

    commit();

    return movies.values().toVector();
}

void Database::clearAllConcerts()
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM concerts");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='concerts'");
    query.exec();
    query.prepare("DELETE FROM concertFiles");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='concertFiles'");
    query.exec();
}

void Database::clearConcertsInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM concertFiles WHERE idConcert IN (SELECT idConcert FROM concerts WHERE path=:path)");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    query.prepare("DELETE FROM concerts WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
}

void Database::add(Concert* concert, DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO concerts(content, inSeparateFolder, path) "
                  "VALUES(:content, :inSeparateFolder, :path)");
    query.bindValue(":content", concert->nfoContent().isEmpty() ? "" : concert->nfoContent().toUtf8());
    query.bindValue(":inSeparateFolder", (concert->inSeparateFolder() ? 1 : 0));
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    int insertId = query.lastInsertId().toInt();

    for (const FilePath& file : concert->files()) {
        query.prepare("INSERT INTO concertFiles(idConcert, file) VALUES(:idConcert, :file)");
        query.bindValue(":idConcert", insertId);
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }
    concert->setDatabaseId(insertId);
}

void Database::update(Concert* concert)
{
    QSqlQuery query(db());
    query.prepare("UPDATE concerts SET content=:content WHERE idConcert=:id");
    query.bindValue(":content", concert->nfoContent().isEmpty() ? "" : concert->nfoContent());
    query.bindValue(":id", concert->databaseId());
    query.exec();

    query.prepare("DELETE FROM concertFiles WHERE idConcert=:idConcert");
    query.bindValue(":idConcert", concert->databaseId());
    query.exec();
    for (const FilePath& file : concert->files()) {
        query.prepare("INSERT INTO concertFiles(idConcert, file) VALUES(:idConcert, :file)");
        query.bindValue(":idConcert", concert->databaseId());
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }
}

QVector<Concert*> Database::concertsInDirectory(DirectoryPath path)
{
    QVector<Concert*> concerts;
    QSqlQuery query(db());
    QSqlQuery queryFiles(db());
    query.prepare("SELECT idConcert, content, inSeparateFolder FROM concerts WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    while (query.next()) {
        QStringList files;
        queryFiles.prepare("SELECT file FROM concertFiles WHERE idConcert=:idConcert");
        queryFiles.bindValue(":idConcert", query.value(query.record().indexOf("idConcert")).toInt());
        queryFiles.exec();
        while (queryFiles.next()) {
            files << QString::fromUtf8(queryFiles.value(queryFiles.record().indexOf("file")).toByteArray());
        }

        auto* concert = new Concert(files, Manager::instance()->concertFileSearcher());
        concert->setDatabaseId(query.value(query.record().indexOf("idConcert")).toInt());
        concert->setInSeparateFolder(query.value(query.record().indexOf("inSeparateFolder")).toInt() == 1);
        concert->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        concerts.append(concert);
    }
    return concerts;
}

void Database::add(TvShow* show, DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO shows(dir, content, path) "
                  "VALUES(:dir, :content, :path)");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.bindValue(":content", show->nfoContent().isEmpty() ? "" : show->nfoContent().toUtf8());
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    show->setDatabaseId(query.lastInsertId().toInt());

    query.prepare("SELECT showMissingEpisodes, hideSpecialsInMissingEpisodes FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.exec();
    if (query.next()) {
        show->setShowMissingEpisodes(query.value(query.record().indexOf("showMissingEpisodes")).toInt() == 1);
        show->setHideSpecialsInMissingEpisodes(
            query.value(query.record().indexOf("hideSpecialsInMissingEpisodes")).toInt() == 1);
    } else {
        query.prepare("INSERT INTO showsSettings(showMissingEpisodes, hideSpecialsInMissingEpisodes, dir, tvdbid, url) "
                      "VALUES(0, 0, :dir, :tvdbid, :url)");
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":tvdbid", show->tvdbId().toString());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.exec();
        show->setShowMissingEpisodes(false);
        show->setHideSpecialsInMissingEpisodes(false);
    }
}

void Database::setShowMissingEpisodes(TvShow* show, bool showMissing)
{
    QSqlQuery query(db());

    query.prepare("SELECT showMissingEpisodes FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.exec();
    if (query.next()) {
        query.prepare("UPDATE showsSettings SET showMissingEpisodes=:show, url=:url, tvdbid=:tvdbid WHERE dir=:dir");
        query.bindValue(":show", showMissing ? 1 : 0);
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":tvdbid", show->tvdbId().toString());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.exec();
    } else {
        query.prepare(
            "INSERT INTO showsSettings(showMissingEpisodes, dir, tvdbid, url) VALUES(:show, :dir, :tvdbid, :url)");
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.bindValue(":tvdbid", show->tvdbId().toString());
        query.bindValue(":show", showMissing ? 1 : 0);
        query.exec();
    }
}

void Database::setHideSpecialsInMissingEpisodes(TvShow* show, bool hideSpecials)
{
    QSqlQuery query(db());

    query.prepare("SELECT hideSpecialsInMissingEpisodes FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.exec();
    if (query.next()) {
        query.prepare(
            "UPDATE showsSettings SET hideSpecialsInMissingEpisodes=:hide, url=:url, tvdbid=:tvdbid WHERE dir=:dir");
        query.bindValue(":show", hideSpecials ? 1 : 0);
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":tvdbid", show->tvdbId().toString());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.exec();
    } else {
        query.prepare("INSERT INTO showsSettings(hideSpecialsInMissingEpisodes, dir, tvdbid, url) VALUES(:hide, :dir, "
                      ":tvdbid, :url)");
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
        query.bindValue(":tvdbid", show->tvdbId().toString());
        query.bindValue(":hide", hideSpecials ? 1 : 0);
        query.exec();
    }
}

void Database::add(TvShowEpisode* episode, DirectoryPath path, int idShow)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO episodes(content, idShow, path, seasonNumber, episodeNumber) "
                  "VALUES(:content, :idShow, :path, :seasonNumber, :episodeNumber)");
    query.bindValue(":content", episode->nfoContent().isEmpty() ? "" : episode->nfoContent().toUtf8());
    query.bindValue(":idShow", idShow);
    query.bindValue(":path", path.toString().toUtf8());
    query.bindValue(":seasonNumber", episode->seasonNumber().toInt());
    query.bindValue(":episodeNumber", episode->episodeNumber().toInt());
    query.exec();
    int insertId = query.lastInsertId().toInt();
    for (const FilePath& file : episode->files()) {
        query.prepare("INSERT INTO episodeFiles(idEpisode, file) VALUES(:idEpisode, :file)");
        query.bindValue(":idEpisode", insertId);
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }
    episode->setDatabaseId(insertId);
}

void Database::update(TvShow* show)
{
    QSqlQuery query(db());
    query.prepare("UPDATE shows SET content=:content, dir=:dir WHERE idShow=:id");
    query.bindValue(":content", show->nfoContent().isEmpty() ? "" : show->nfoContent());
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.bindValue(":id", show->databaseId());
    query.exec();

    int id = showsSettingsId(show);
    query.prepare("UPDATE showsSettings SET showMissingEpisodes=:show, hideSpecialsInMissingEpisodes=:hide, url=:url, "
                  "tvdbid=:tvdbid WHERE idShow=:idShow");
    query.bindValue(":show", show->showMissingEpisodes());
    query.bindValue(":hide", show->hideSpecialsInMissingEpisodes());
    query.bindValue(":idShow", id);
    query.bindValue(":tvdbid", show->tvdbId().toString());
    query.bindValue(":url", show->episodeGuideUrl().isEmpty() ? "" : show->episodeGuideUrl());
    query.exec();
}

void Database::update(TvShowEpisode* episode)
{
    QSqlQuery query(db());
    query.prepare("UPDATE episodes SET content=:content WHERE idEpisode=:id");
    query.bindValue(":content", episode->nfoContent().isEmpty() ? "" : episode->nfoContent());
    query.bindValue(":id", episode->databaseId());
    query.exec();

    query.prepare("DELETE FROM episodeFiles WHERE idEpisode=:idEpisode");
    query.bindValue(":idEpisode", episode->databaseId());
    query.exec();

    for (const FilePath& file : episode->files()) {
        query.prepare("INSERT INTO episodeFiles(idEpisode, file) VALUES(:idEpisode, :file)");
        query.bindValue(":idEpisode", episode->databaseId());
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }
}

int Database::showCount(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("SELECT COUNT(*) FROM shows WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    if (!query.next()) {
        return 0;
    }
    bool ok = false;
    int numberOfShows = query.value(0).toInt(&ok);
    return ok ? numberOfShows : 0;
}

QVector<TvShow*> Database::showsInDirectory(DirectoryPath path)
{
    QVector<TvShow*> shows;
    QSqlQuery query(db());
    query.prepare("SELECT idShow, dir, content, path FROM shows WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    while (query.next()) {
        auto* show = new TvShow(QString::fromUtf8(query.value(query.record().indexOf("dir")).toByteArray()),
            Manager::instance()->tvShowFileSearcher());
        show->setDatabaseId(query.value(query.record().indexOf("idShow")).toInt());
        show->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        shows.append(show);
    }

    for (TvShow* show : shows) {
        query.prepare("SELECT showMissingEpisodes, hideSpecialsInMissingEpisodes FROM showsSettings WHERE dir=:dir");
        query.bindValue(":dir", show->dir().toString().toUtf8());
        query.exec();
        if (query.next()) {
            show->setShowMissingEpisodes(
                query.value(query.record().indexOf("showMissingEpisodes")).toInt() == 1, false);
            show->setHideSpecialsInMissingEpisodes(
                query.value(query.record().indexOf("hideSpecialsInMissingEpisodes")).toInt() == 1, false);
        }
    }

    return shows;
}

QVector<TvShowEpisode*> Database::episodes(int idShow)
{
    QVector<TvShowEpisode*> episodes;
    QSqlQuery query(db());
    QSqlQuery queryFiles(db());
    query.prepare("SELECT idEpisode, content, seasonNumber, episodeNumber FROM episodes WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();
    while (query.next()) {
        QStringList files;
        queryFiles.prepare("SELECT file FROM episodeFiles WHERE idEpisode=:idEpisode");
        queryFiles.bindValue(":idEpisode", query.value(query.record().indexOf("idEpisode")).toInt());
        queryFiles.exec();
        while (queryFiles.next()) {
            files << QString::fromUtf8(queryFiles.value(queryFiles.record().indexOf("file")).toByteArray());
        }

        auto* episode = new TvShowEpisode(files);
        episode->setSeason(SeasonNumber(query.value(query.record().indexOf("seasonNumber")).toInt()));
        episode->setEpisode(EpisodeNumber(query.value(query.record().indexOf("episodeNumber")).toInt()));
        episode->setDatabaseId(query.value(query.record().indexOf("idEpisode")).toInt());
        episode->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        episodes.append(episode);
    }
    return episodes;
}

void Database::clearAllTvShows()
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM shows");
    query.exec();
    query.prepare("DELETE FROM episodes");
    query.exec();
    query.prepare("DELETE FROM episodeFiles");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='shows'");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='episodes'");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='episodeFiles'");
    query.exec();
}

void Database::clearTvShowsInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM shows WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    query.prepare("DELETE FROM episodeFiles WHERE idEpisode IN (SELECT idEpisode FROM episodes WHERE path=:path)");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    query.prepare("DELETE FROM episodes WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
}

void Database::clearTvShowInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("SELECT idShow FROM shows WHERE dir=:dir");
    query.bindValue(":dir", path.toString().toUtf8());
    query.exec();
    if (!query.next()) {
        return;
    }
    int idShow = query.value(0).toInt();

    query.prepare("DELETE FROM episodeFiles WHERE idEpisode IN (SELECT idEpisode FROM episodes WHERE idShow=:idShow)");
    query.bindValue(":idShow", idShow);
    query.exec();

    query.prepare("DELETE FROM shows WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();

    query.prepare("DELETE FROM episodes WHERE idShow=:idShow");
    query.bindValue(":idShow", idShow);
    query.exec();
}

int Database::episodeCount()
{
    QSqlQuery query(db());
    query.prepare("SELECT COUNT(*) FROM episodes");
    query.exec();
    query.next();
    return query.value(0).toInt();
}

int Database::showsSettingsId(TvShow* show)
{
    QSqlQuery query(db());
    query.prepare("SELECT idShow FROM showsSettings WHERE dir=:dir");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.exec();
    if (query.next()) {
        return query.value(0).toInt();
    }

    query.prepare("INSERT INTO showsSettings(showMissingEpisodes, hideSpecialsInMissingEpisodes, dir) VALUES(:show, "
                  ":hide, :dir)");
    query.bindValue(":dir", show->dir().toString().toUtf8());
    query.bindValue(":show", 0);
    query.bindValue(":hide", 0);
    query.exec();
    return query.lastInsertId().toInt();
}

void Database::clearEpisodeList(int showsSettingsId)
{
    QSqlQuery query(db());
    query.prepare("UPDATE showsEpisodes SET updated=0 WHERE idShow=:idShow");
    query.bindValue(":idShow", showsSettingsId);
    query.exec();
}

void Database::addEpisodeToShowList(TvShowEpisode* episode, int showsSettingsId, TvDbId tvdbid)
{
    kodi::EpisodeXmlWriterGeneric xmlWriter(KodiVersion::latest(), {episode});
    const QByteArray xmlContent = xmlWriter.getEpisodeXml();

    QSqlQuery query(db());
    query.prepare("SELECT idEpisode FROM showsEpisodes WHERE tvdbid=:tvdbid");
    query.bindValue(":tvdbid", tvdbid.toString());
    query.exec();
    if (query.next()) {
        const int idEpisode = query.value(0).toInt();
        query.prepare("UPDATE showsEpisodes SET seasonNumber=:seasonNumber, episodeNumber=:episodeNumber, updated=1, "
                      "content=:content WHERE idEpisode=:idEpisode");
        query.bindValue(":content", xmlContent.isEmpty() ? "" : xmlContent);
        query.bindValue(":idEpisode", idEpisode);
        query.bindValue(":seasonNumber", episode->seasonNumber().toInt());
        query.bindValue(":episodeNumber", episode->episodeNumber().toInt());
        query.exec();
    } else {
        query.prepare("INSERT INTO showsEpisodes(content, idShow, seasonNumber, episodeNumber, tvdbid, updated) "
                      "VALUES(:content, :idShow, :seasonNumber, :episodeNumber, :tvdbid, 1)");
        query.bindValue(":content", xmlContent.isEmpty() ? "" : xmlContent);
        query.bindValue(":idShow", showsSettingsId);
        query.bindValue(":seasonNumber", episode->seasonNumber().toInt());
        query.bindValue(":episodeNumber", episode->episodeNumber().toInt());
        query.bindValue(":tvdbid", tvdbid.toString());
        query.exec();
    }
}

void Database::cleanUpEpisodeList(int showsSettingsId)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM showsEpisodes WHERE idShow=:idShow AND updated=0");
    query.bindValue(":idShow", showsSettingsId);
    query.exec();
}

QVector<TvShowEpisode*> Database::showsEpisodes(TvShow* show)
{
    int id = showsSettingsId(show);
    QVector<TvShowEpisode*> episodes;
    QSqlQuery query(db());
    query.prepare("SELECT idEpisode, content, seasonNumber, episodeNumber FROM showsEpisodes WHERE idShow=:idShow");
    query.bindValue(":idShow", id);
    query.exec();
    while (query.next()) {
        auto* episode = new TvShowEpisode(QStringList(), show);
        episode->setSeason(SeasonNumber(query.value(query.record().indexOf("seasonNumber")).toInt()));
        episode->setEpisode(EpisodeNumber(query.value(query.record().indexOf("episodeNumber")).toInt()));
        episode->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        episodes.append(episode);
    }
    return episodes;
}

void Database::addImport(QString fileName, QString type, DirectoryPath path)
{
    int id = 1;
    QSqlQuery query(db());
    query.prepare("SELECT MAX(id) FROM importCache");
    query.exec();
    if (query.next()) {
        id = query.value(0).toInt() + 1;
    }

    query.prepare("INSERT INTO importCache(id, filename, type, path) VALUES(:id, :filename, :type, :path)");
    query.bindValue(":id", id);
    query.bindValue(":filename", fileName);
    query.bindValue(":type", type);
    query.bindValue(":path", path.toString());
    query.exec();
}

bool Database::guessImport(QString fileName, QString& type, QString& path)
{
    qreal bestMatch = 0;

    QSqlQuery query(db());
    query.prepare("SELECT filename, type, path FROM importCache");
    query.exec();
    while (query.next()) {
        qreal p = helper::similarity(fileName, query.value(query.record().indexOf("filename")).toString());
        if (p > 0.7 && p > bestMatch) {
            bestMatch = p;
            type = query.value(query.record().indexOf("type")).toString();
            path = query.value(query.record().indexOf("path")).toString();
        }
    }

    return (bestMatch != 0);
}

void Database::setLabel(const mediaelch::FileList& fileNames, ColorLabel colorLabel)
{
    // no locker, as this function is called by add()

    int color = static_cast<int>(colorLabel);
    QSqlQuery query(db());
    int id = 1;
    query.prepare("SELECT MAX(idLabel) FROM labels");
    query.exec();
    if (query.next()) {
        id = query.value(0).toInt() + 1;
    }

    for (const mediaelch::FilePath& fileName : fileNames) {
        query.prepare("SELECT idLabel FROM labels WHERE fileName=:fileName");
        query.bindValue(":fileName", fileName.toString().toUtf8());
        query.exec();
        if (query.next()) {
            int idLabel = query.value(query.record().indexOf("idLabel")).toInt();
            query.prepare("UPDATE labels SET color=:color WHERE idLabel=:idLabel");
            query.bindValue(":idLabel", idLabel);
            query.bindValue(":color", color);
            query.exec();
        } else {
            query.prepare("INSERT INTO labels(idLabel, color, fileName) VALUES(:idLabel, :color, :fileName)");
            query.bindValue(":idLabel", id);
            query.bindValue(":color", color);
            query.bindValue(":fileName", fileName.toString().toUtf8());
            query.exec();
        }
    }
}

ColorLabel Database::getLabel(const mediaelch::FileList& fileNames)
{
    QMutexLocker locker(&m_mutex);

    if (fileNames.isEmpty()) {
        return ColorLabel::NoLabel;
    }

    QSqlQuery query(db());
    query.prepare("SELECT color FROM labels WHERE fileName=:fileName");
    query.bindValue(":fileName", fileNames.first().toString().toUtf8());
    bool success = query.exec();
    if (success && query.next()) {
        return static_cast<ColorLabel>(query.value("color").toInt());
    }

    return ColorLabel::NoLabel;
}

void Database::clearAllArtists()
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM artists");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='artists'");
    query.exec();
    clearAllAlbums();
}

void Database::clearArtistsInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM artists WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    clearAlbumsInDirectory(path);
}

void Database::add(Artist* artist, DirectoryPath path)
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

void Database::update(Artist* artist)
{
    QSqlQuery query(db());
    query.prepare("UPDATE artists SET content=:content WHERE idArtist=:id");
    query.bindValue(":content", artist->nfoContent().isEmpty() ? "" : artist->nfoContent());
    query.bindValue(":id", artist->databaseId());
    query.exec();
}

QVector<Artist*> Database::artistsInDirectory(DirectoryPath path)
{
    QVector<Artist*> artists;
    QSqlQuery query(db());
    query.prepare("SELECT idArtist, content, dir FROM artists WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    while (query.next()) {
        auto* artist = new Artist(QString::fromUtf8(query.value(query.record().indexOf("dir")).toByteArray()),
            Manager::instance()->musicFileSearcher());
        artist->setDatabaseId(query.value(query.record().indexOf("idArtist")).toInt());
        artist->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        artists.append(artist);
    }
    return artists;
}

void Database::clearAllAlbums()
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM albums");
    query.exec();
    query.prepare("DELETE FROM sqlite_sequence WHERE name='albums'");
    query.exec();
}

void Database::clearAlbumsInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM albums WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
}

void Database::add(Album* album, DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("INSERT INTO albums(idArtist, content, dir, path) "
                  "VALUES(:idArtist, :content, :dir, :path)");
    query.bindValue(":idArtist", album->artistObj()->databaseId());
    query.bindValue(":content", album->nfoContent().isEmpty() ? "" : album->nfoContent().toUtf8());
    query.bindValue(":dir", album->path().toString().toUtf8());
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    album->setDatabaseId(query.lastInsertId().toInt());
}

void Database::update(Album* album)
{
    QSqlQuery query(db());
    query.prepare("UPDATE albums SET content=:content WHERE idAlbum=:id");
    query.bindValue(":content", album->nfoContent().isEmpty() ? "" : album->nfoContent());
    query.bindValue(":id", album->databaseId());
    query.exec();
}

QVector<Album*> Database::albums(Artist* artist)
{
    QVector<Album*> albums;
    QSqlQuery query(db());
    query.prepare("SELECT idAlbum, content, dir FROM albums WHERE idArtist=:idArtist");
    query.bindValue(":idArtist", artist->databaseId());
    query.exec();
    while (query.next()) {
        auto* album = new Album(QString::fromUtf8(query.value(query.record().indexOf("dir")).toByteArray()),
            Manager::instance()->musicFileSearcher());
        album->setDatabaseId(query.value(query.record().indexOf("idAlbum")).toInt());
        album->setNfoContent(QString::fromUtf8(query.value(query.record().indexOf("content")).toByteArray()));
        album->setArtistObj(artist);
        artist->addAlbum(album);
        albums.append(album);
    }
    return albums;
}
