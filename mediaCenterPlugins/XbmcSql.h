#ifndef XBMCSQL_H
#define XBMCSQL_H

#include <QObject>
#include <QSqlDatabase>

#include "data/MediaCenterInterface.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

class XbmcSql : public MediaCenterInterface
{
    Q_OBJECT
public:
    explicit XbmcSql(QObject *parent = 0);
    ~XbmcSql();

    bool saveMovie(Movie *movie);
    bool loadMovie(Movie *movie);
    void loadMovieImages(Movie *movie);
    void exportDatabase(QList<Movie*> movies, QList<TvShow*> shows, QString exportPath, QString pathSearch, QString pathReplace);
    bool loadTvShow(TvShow *show);
    void loadTvShowImages(TvShow *show);
    bool loadTvShowEpisode(TvShowEpisode *episode);
    void loadTvShowEpisodeImages(TvShowEpisode *episode);
    bool saveTvShow(TvShow *show);
    bool saveTvShowEpisode(TvShowEpisode *episode);
    void connectMysql(QString host, QString database, QString username, QString password);
    void connectSqlite(QString database);
    void shutdown();
    bool hasFeature(int feature);

signals:
    void sigExportStarted();
    void sigExportProgress(int, int);
    void sigExportDone();
    void sigExportRaiseError(QString);

private:
    QString hash(QString string);
    QString actorHash(Actor actor);
    QString mediaCenterPath(QString file);
    QString mediaCenterDir(QString file);
    QString tvShowMediaCenterPath(QString file);
    QString tvShowMediaCenterDir(QString file);
    QSqlDatabase db();

    QSqlDatabase *m_db;
    bool m_isMySQL;
};

#endif // XBMCSQL_H
