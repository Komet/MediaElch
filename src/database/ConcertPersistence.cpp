#include "database/ConcertPersistence.h"

#include "data/concert/Concert.h"
#include "database/Database.h"
#include "globals/Manager.h"
#include "log/Log.h"

#include <QSqlQuery>
#include <QSqlRecord>

namespace mediaelch {

ConcertPersistence::ConcertPersistence(Database& db) : m_db(db)
{
}

QSqlDatabase ConcertPersistence::db()
{
    return m_db.db();
}

void ConcertPersistence::clearAllConcerts()
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

void ConcertPersistence::clearConcertsInDirectory(DirectoryPath path)
{
    QSqlQuery query(db());
    query.prepare("DELETE FROM concertFiles WHERE idConcert IN (SELECT idConcert FROM concerts WHERE path=:path)");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
    query.prepare("DELETE FROM concerts WHERE path=:path");
    query.bindValue(":path", path.toString().toUtf8());
    query.exec();
}

void ConcertPersistence::add(Concert* concert, DirectoryPath path)
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

void ConcertPersistence::update(Concert* concert)
{
    QSqlQuery query(db());
    query.prepare("UPDATE concerts SET content=:content WHERE idConcert=:id");
    query.bindValue(":content", concert->nfoContent().isEmpty() ? "" : concert->nfoContent());
    query.bindValue(":id", concert->databaseId().toInt());
    query.exec();

    query.prepare("DELETE FROM concertFiles WHERE idConcert=:idConcert");
    query.bindValue(":idConcert", concert->databaseId().toInt());
    query.exec();
    for (const FilePath& file : concert->files()) {
        query.prepare("INSERT INTO concertFiles(idConcert, file) VALUES(:idConcert, :file)");
        query.bindValue(":idConcert", concert->databaseId().toInt());
        query.bindValue(":file", file.toString().toUtf8());
        query.exec();
    }
}

QVector<Concert*> ConcertPersistence::concertsInDirectory(DirectoryPath path)
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


} // namespace mediaelch
