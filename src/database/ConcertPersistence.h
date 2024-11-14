#pragma once

#include "media/Path.h"

#include <QObject>
#include <QSqlDatabase>

class Concert;
class Database;

namespace mediaelch {

class ConcertPersistence
{
public:
    explicit ConcertPersistence(Database& db);
    QSqlDatabase db();

    void clearAllConcerts();
    void clearConcertsInDirectory(DirectoryPath path);
    void add(Concert* concert, DirectoryPath path);
    void update(Concert* concert);
    QVector<Concert*> concertsInDirectory(DirectoryPath path);


private:
    Database& m_db;
};

} // namespace mediaelch
