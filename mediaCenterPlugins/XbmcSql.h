#ifndef XBMCSQL_H
#define XBMCSQL_H

#include <QObject>
#include <QSqlDatabase>

#include "data/Concert.h"
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
    bool loadMovie(Movie *movie, QString nfoContent = "");
    bool saveConcert(Concert *concert);
    bool loadConcert(Concert *concert, QString nfoContent = "");
    bool loadTvShow(TvShow *show, QString nfoContent = "");
    bool loadTvShowEpisode(TvShowEpisode *episode, QString nfoContent = "");
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
    QString posterImageName(Movie *movie);
    QString backdropImageName(Movie *movie);
    QString logoImageName(Movie *movie);
    QString clearArtImageName(Movie *movie);
    QString cdArtImageName(Movie *movie);
    QString actorImageName(Movie *movie, Actor actor);
    QString posterImageName(Concert *concert);
    QString backdropImageName(Concert *concert);
    QString logoImageName(Concert *concert);
    QString clearArtImageName(Concert *concert);
    QString cdArtImageName(Concert *concert);
    QString thumbnailImageName(TvShowEpisode *episode);
    QString posterImageName(TvShow *show);
    QString backdropImageName(TvShow *show);
    QString bannerImageName(TvShow *show);
    QString actorImageName(TvShow *show, Actor actor);
    QString seasonPosterImageName(TvShow *show, int season);
    QString logoImageName(TvShow *show);
    QString clearArtImageName(TvShow *show);
    QString characterArtImageName(TvShow *show);

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
