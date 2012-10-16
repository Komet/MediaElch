#ifndef IMAGEPROVIDERINTERFACE_H
#define IMAGEPROVIDERINTERFACE_H

#include <QObject>
#include "globals/Globals.h"

/**
 * @brief The ImageProviderInterface class
 */
class ImageProviderInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual void moviePosters(QString tmdbId) = 0;
    virtual void movieBackdrops(QString tmdbId) = 0;
    virtual void movieLogos(QString tmdbId) = 0;
    virtual void movieClearArts(QString tmdbId) = 0;
    virtual void movieCdArts(QString tmdbId) = 0;
    virtual QList<int> provides() = 0;
public slots:
    virtual void searchMovie(QString searchStr, int limit) = 0;
signals:
    virtual void sigSearchDone(QList<ScraperSearchResult>) = 0;
    virtual void sigImagesLoaded(QList<Poster>) = 0;

};

#endif // IMAGEPROVIDERINTERFACE_H
