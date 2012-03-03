#ifndef MEDIACENTERINTERFACE_H
#define MEDIACENTERINTERFACE_H

#include "data/Movie.h"

class Movie;

class MediaCenterInterface
{
public:
    virtual bool saveData(Movie *movie) = 0;
    virtual bool loadData(Movie *movie) = 0;
    virtual void loadImages(Movie *movie) = 0;
};

#endif // MEDIACENTERINTERFACE_H
