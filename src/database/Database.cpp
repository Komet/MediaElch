#include "Database.h"

#include "data/Subtitle.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "settings/Settings.h"

#include <QDir>
#include <QMutexLocker>
#include <QSqlQuery>
#include <QSqlRecord>

using namespace mediaelch;

/// \brief This mutex is used for initializing new connections.
static QMutex s_initializingDatabaseMutex;
/// \brief Used for creating a new connection name.
static size_t s_connectionCount = 0;

Database::Database(QObject* parent) : QObject(parent)
{
    // This lock is required to ensure that multithreaded access only initializes
    // the database once.  Each instance of this class has its own connection name.
    QMutexLocker lock(&s_initializingDatabaseMutex);
    ++s_connectionCount;

    m_dataLocation = Settings::instance()->databaseDir();
    QDir dir(m_dataLocation.dir());
    if (!dir.exists()) {
        dir.mkpath(m_dataLocation.toString());
    }

    QString connectionName = QStringLiteral("mediaDb_%1").arg(s_connectionCount);
    m_db = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE", connectionName));
    m_db->setDatabaseName(m_dataLocation.filePath("MediaElch.sqlite"));
    if (!m_db->open()) {
        qCCritical(generic) << "Could not open cache database";
    } else {
        setupDatabase();
    }
}

Database::~Database()
{
    if (m_db != nullptr && m_db->isOpen()) {
        m_db->close();
    }
}

Database* Database::newConnection(QObject* parent)
{
    return new Database(parent);
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

QSqlDatabase Database::db()
{
    return *m_db;
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

void Database::setupDatabase()
{
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
        updateDbVersion(16);
    }

    if (myDbVersion < 17) {
        query.prepare("DROP TABLE IF EXISTS showsEpisodes;");
        query.exec();

        query.prepare("DROP TABLE IF EXISTS showsSettings;");
        query.exec();

        query.prepare(R"sql(CREATE TABLE IF NOT EXISTS showsEpisodes (
              "idEpisode" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
              "content" text NOT NULL,
              "idShow" integer NOT NULL,
              "seasonNumber" integer NOT NULL,
              "episodeNumber" integer NOT NULL,
              "tmdbid" text NOT NULL,
              "updated" integer NOT NULL);
        )sql");
        query.exec();

        query.prepare(R"sql(CREATE TABLE IF NOT EXISTS showsSettings (
                      "idShow" integer NOT NULL PRIMARY KEY AUTOINCREMENT,
                      "tmdbid" text NOT NULL,
                      "url" text NOT NULL,
                      "showMissingEpisodes" integer NOT NULL,
                      "hideSpecialsInMissingEpisodes" integer NOT NULL,
                      "dir" text NOT NULL);
        )sql");
        query.exec();

        myDbVersion = 17;
        Q_UNUSED(myDbVersion);
        updateDbVersion(17);
    }

    query.prepare("PRAGMA synchronous=0;");
    query.exec();

    query.prepare("PRAGMA journal_mode=WAL;");
    query.exec();

    query.prepare("PRAGMA cache_size=20000;");
    query.exec();
}
