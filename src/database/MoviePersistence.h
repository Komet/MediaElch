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
    virtual ~MoviePersistence() = default;
    QSqlDatabase db();

    virtual void clearAllMovies();
    virtual void clearMoviesInDirectory(DirectoryPath path);
    virtual void addMovie(Movie* movie, DirectoryPath path);
    virtual void update(Movie* movie);
    virtual QVector<Movie*> moviesInDirectory(DirectoryPath path, QObject* movieParent);

private:
    Database& m_db;
};

} // namespace mediaelch
