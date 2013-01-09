#ifndef DATABASE_H
#define DATABASE_H

#include <QDateTime>
#include <QObject>
#include <QSqlDatabase>
#include "data/Concert.h"
#include "movies/Movie.h"
#include "data/TvShow.h"

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = 0);
    ~Database();
    QSqlDatabase db();
    void transaction();
    void commit();
    void clearMovies(QString path = "");
    void add(Movie *movie, QString path);
    void update(Movie *movie);
    QList<Movie*> movies(QString path);

    void clearConcerts(QString path = "");
    void add(Concert *concert, QString path);
    void update(Concert *concert);
    QList<Concert*> concerts(QString path);

    void add(TvShow *show, QString path);
    void add(TvShowEpisode *episode, QString path, int idShow);
    void update(TvShow *show);
    void update(TvShowEpisode *episode);
    void clearTvShows(QString path = "");
    void clearTvShow(QString showDir);
    QList<TvShow*> shows(QString path);
    QList<TvShowEpisode*> episodes(int idShow);
    int episodeCount();
private:
    QSqlDatabase *m_db;
};

#endif // DATABASE_H
