#ifndef XBMCSQL_H
#define XBMCSQL_H

#include <QObject>
#include <QSqlDatabase>

#include "data/MediaCenterInterface.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

/**
 * @brief The XbmcSql class
 */
class XbmcSql : public MediaCenterInterface
{
    Q_OBJECT
public:
    explicit XbmcSql(QObject *parent = 0, QString connectionName = "xbmc");
    ~XbmcSql();

    bool saveMovie(Movie *movie);
    bool loadMovie(Movie *movie);
    void loadMovieImages(Movie *movie);
    bool saveConcert(Concert *concert);
    bool loadConcert(Concert *concert);
    void loadConcertImages(Concert *concert);
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
    QImage movieSetPoster(QString setName);
    QImage movieSetBackdrop(QString setName);
    void saveMovieSetPoster(QString setName, QImage poster);
    void saveMovieSetBackdrop(QString setName, QImage backdrop);

signals:
    void sigExportStarted();
    void sigExportProgress(int, int);
    void sigExportDone();
    void sigExportRaiseError(QString);

private:
    QString hash(QString string);
    QString actorHash(Actor actor);
    QString movieSetHash(QString setName);
    QString mediaCenterPath(QString file);
    QString mediaCenterDir(QString file);
    QString tvShowMediaCenterPath(QString file);
    QString tvShowMediaCenterDir(QString file);
    QString concertMediaCenterPath(QString file);
    QString concertMediaCenterDir(QString file);
    QSqlDatabase db();

    QSqlDatabase *m_db;
    QString m_connectionName;
    bool m_isMySQL;
};

#endif // XBMCSQL_H
