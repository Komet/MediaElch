#ifndef MEDIACENTERINTERFACE_H
#define MEDIACENTERINTERFACE_H

#include "globals/Globals.h"
#include "data/Concert.h"
#include "movies/Movie.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"
#include "settings/DataFile.h"

class Album;
class Artist;
class Concert;
class Movie;
class TvShow;
class TvShowEpisode;

/**
 * @brief The MediaCenterInterface class
 * This class is the base for every MediaCenter.
 */
class MediaCenterInterface : public QObject
{
public:
    virtual bool saveMovie(Movie *movie) = 0;
    virtual bool loadMovie(Movie *movie, QString nfoContent = "") = 0;
    virtual bool saveConcert(Concert *concert) = 0;
    virtual bool loadConcert(Concert *concert, QString nfoContent = "") = 0;
    virtual bool loadTvShow(TvShow *show, QString nfoContent = "") = 0;
    virtual bool loadTvShowEpisode(TvShowEpisode *episode, QString nfoContent = "") = 0;
    virtual QImage movieSetPoster(QString setName) = 0;
    virtual QImage movieSetBackdrop(QString setName) = 0;
    virtual void saveMovieSetPoster(QString setName, QImage poster) = 0;
    virtual void saveMovieSetBackdrop(QString setName, QImage backdrop) = 0;
    virtual bool saveTvShow(TvShow *show) = 0;
    virtual bool saveTvShowEpisode(TvShowEpisode *episode) = 0;
    virtual bool hasFeature(int feature) = 0;
    virtual QStringList extraFanartNames(Movie *movie) = 0;
    virtual QStringList extraFanartNames(TvShow *show) = 0;
    virtual QStringList extraFanartNames(Concert *concert) = 0;
    virtual QStringList extraFanartNames(Artist *artist) = 0;

    virtual bool saveArtist(Artist *artist) = 0;
    virtual bool saveAlbum(Album *album) = 0;
    virtual bool loadArtist(Artist *artist, QString initialNfoContent = "") = 0;
    virtual bool loadAlbum(Album *album, QString initialNfoContent = "") = 0;

    virtual QString actorImageName(Movie *movie, Actor actor) = 0;
    virtual QString actorImageName(TvShow *show, Actor actor) = 0;
    virtual QString actorImageName(TvShowEpisode *episode, Actor actor) = 0;

    virtual QString nfoFilePath(Movie *movie) = 0;
    virtual QString nfoFilePath(Concert *concert) = 0;
    virtual QString nfoFilePath(TvShowEpisode *episode) = 0;
    virtual QString nfoFilePath(TvShow *show) = 0;
    virtual QString nfoFilePath(Artist *artist) = 0;
    virtual QString nfoFilePath(Album *album) = 0;

    virtual QString imageFileName(Movie *movie, int type, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false) = 0;
    virtual QString imageFileName(Concert *concert, int type, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false) = 0;
    virtual QString imageFileName(TvShowEpisode *episode, int type, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false) = 0;
    virtual QString imageFileName(TvShow *show, int type, int season = -2, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false) = 0;
    virtual QString imageFileName(Artist *artist, int type, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false) = 0;
    virtual QString imageFileName(Album *album, int type, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false) = 0;

    virtual void loadBooklets(Album *album) = 0;
};

#endif // MEDIACENTERINTERFACE_H
