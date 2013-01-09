#ifndef MEDIACENTERINTERFACE_H
#define MEDIACENTERINTERFACE_H

#include "globals/Globals.h"
#include "data/Concert.h"
#include "movies/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

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
    virtual QString posterImageName(Movie *movie) = 0;
    virtual QString backdropImageName(Movie *movie) = 0;
    virtual QString logoImageName(Movie *movie) = 0;
    virtual QString clearArtImageName(Movie *movie) = 0;
    virtual QString cdArtImageName(Movie *movie) = 0;
    virtual QString actorImageName(Movie *movie, Actor actor) = 0;
    virtual QString posterImageName(Concert *concert) = 0;
    virtual QString backdropImageName(Concert *concert) = 0;
    virtual QString logoImageName(Concert *concert) = 0;
    virtual QString clearArtImageName(Concert *concert) = 0;
    virtual QString cdArtImageName(Concert *concert) = 0;
    virtual QString thumbnailImageName(TvShowEpisode *episode) = 0;
    virtual QString posterImageName(TvShow *show) = 0;
    virtual QString backdropImageName(TvShow *show) = 0;
    virtual QString bannerImageName(TvShow *show) = 0;
    virtual QString actorImageName(TvShow *show, Actor actor) = 0;
    virtual QString logoImageName(TvShow *show) = 0;
    virtual QString clearArtImageName(TvShow *show) = 0;
    virtual QString characterArtImageName(TvShow *show) = 0;
    virtual QString seasonPosterImageName(TvShow *show, int season) = 0;
};

#endif // MEDIACENTERINTERFACE_H
