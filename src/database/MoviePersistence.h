#pragma once

#include "media/Path.h"

#include <QObject>
#include <QSqlDatabase>

class Movie;
class Database;

namespace mediaelch {

class MoviePersistence
{
public:
    explicit MoviePersistence(Database& db);
    QSqlDatabase db();

    void clearAllMovies();
    void clearMoviesInDirectory(DirectoryPath path);
    void addMovie(Movie* movie, DirectoryPath path);
    void update(Movie* movie);
    QVector<Movie*> moviesInDirectory(DirectoryPath path, QObject* movieParent);

private:
    Database& m_db;
};

} // namespace mediaelch
