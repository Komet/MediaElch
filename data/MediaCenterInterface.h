#ifndef MEDIACENTERINTERFACE_H
#define MEDIACENTERINTERFACE_H

#include "data/Movie.h"

class Movie;

class MediaCenterInterface : public QObject
{
public:
    virtual bool saveData(Movie *movie) = 0;
    virtual bool loadData(Movie *movie) = 0;
    virtual void loadImages(Movie *movie) = 0;
    virtual void exportDatabase(QList<Movie*> movies, QString exportPath, QString pathSearch, QString pathReplace) = 0;
signals:
    virtual void sigExportStarted() = 0;
    virtual void sigExportProgress(int, int) = 0;
    virtual void sigExportDone() = 0;
    virtual void sigExportRaiseError(QString) = 0;
};

#endif // MEDIACENTERINTERFACE_H
