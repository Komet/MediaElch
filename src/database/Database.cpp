#include "Database.h"

#include "data/Subtitle.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "settings/Settings.h"

#include <QDir>
#include <QMutexLocker>
#include <QSqlError>
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

    const QString connectionName = QStringLiteral("mediaDb_%1").arg(s_connectionCount);
    m_db = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE", connectionName));
    const QString dbName = m_dataLocation.filePath("MediaElch.sqlite");
    m_db->setDatabaseName(dbName);

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

QStringList getSqlStatementsFromFile(const QString& filePath)
{
    QFile res(filePath);
    const bool result = res.open(QIODevice::ReadOnly);
    if (!result) {
        qCCritical(generic) << "[Database] Could not open SQL file:" << filePath;
        qFatal("Fatal database error");
    }
    QString fileContents(res.readAll());
    res.close();
    return fileContents.split("-- next", ElchSplitBehavior::SkipEmptyParts);
}

void executeSqlStatementsFromFile(const QString& filePath, QSqlQuery& query)
{
    const QStringList statements = getSqlStatementsFromFile(filePath);
    for (const QString& statement : statements) {
        QString sql = statement.trimmed();
        bool success = query.prepare(sql.trimmed());
        if (!success) {
            qCCritical(generic) << "[Database] Error preparing SQL:" << sql << "; ERROR:" << query.lastError().text();
            qFatal("Fatal database error");
        }
        success = query.exec();
        if (!success) {
            qCCritical(generic) << "[Database] Error executing SQL:" << sql << "; ERROR:" << query.lastError().text();
            qFatal("Fatal database error");
        }
    }
}

void Database::updateDbVersion(int version)
{
    QSqlQuery query(*m_db);

    query.prepare("SELECT * FROM sqlite_master WHERE name ='settings' and type='table';");
    query.exec();
    if (!query.next()) {
        executeSqlStatementsFromFile(":/sql/000_settings.sql", query);
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

void Database::setLabel(const FileList& fileNames, ColorLabel colorLabel)
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

    for (const FilePath& fileName : fileNames) {
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

QString Database::getSchema()
{
    QSqlQuery query(*m_db);
    query.exec("select sql from sqlite_schema;");
    QStringList tables;
    while (query.next()) {
        tables << query.value(0).toString();
    }
    return tables.join(";\n\n");
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
        executeSqlStatementsFromFile(":/sql/014_initial.sql", query);
        myDbVersion = 14;
        updateDbVersion(14);
    }
    if (myDbVersion < 15) {
        executeSqlStatementsFromFile(":/sql/015_artists_albums.sql", query);
        myDbVersion = 15;
        updateDbVersion(15);
    }
    if (myDbVersion < 16) {
        executeSqlStatementsFromFile(":/sql/016_movieSubtitles.sql", query);
        myDbVersion = 16;
        updateDbVersion(16);
    }
    if (myDbVersion < 17) {
        executeSqlStatementsFromFile(":/sql/017_showEpisodes.sql", query);
        myDbVersion = 17;
        updateDbVersion(myDbVersion);
    }
    if (myDbVersion < 18) {
        executeSqlStatementsFromFile(":/sql/018_index_fix.sql", query);
        myDbVersion = 18;
        updateDbVersion(myDbVersion);
    }

    query.prepare("PRAGMA synchronous=0;");
    query.exec();

    query.prepare("PRAGMA journal_mode=WAL;");
    query.exec();

    query.prepare("PRAGMA cache_size=20000;");
    query.exec();
}
