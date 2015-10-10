#ifndef IMAGEPROVIDERINTERFACE_H
#define IMAGEPROVIDERINTERFACE_H

#include <QObject>
#include "data/Concert.h"
#include "movies/Movie.h"
#include "data/TvShow.h"
#include "globals/Globals.h"

class TvShow;

/**
 * @brief The ImageProviderInterface class
 */
class ImageProviderInterface : public QObject
{
public:
    virtual QString name() = 0;
    virtual QString identifier() = 0;
    virtual void movieImages(Movie *movie, QString tmdbId, QList<int> types) = 0;
    virtual void moviePosters(QString tmdbId) = 0;
    virtual void movieBackdrops(QString tmdbId) = 0;
    virtual void movieLogos(QString tmdbId) = 0;
    virtual void movieBanners(QString tmdbId) = 0;
    virtual void movieThumbs(QString tmdbId) = 0;
    virtual void movieClearArts(QString tmdbId) = 0;
    virtual void movieCdArts(QString tmdbId) = 0;
    virtual void concertImages(Concert *concert, QString tmdbId, QList<int> types) = 0;
    virtual void concertPosters(QString tmdbId) = 0;
    virtual void concertBackdrops(QString tmdbId) = 0;
    virtual void concertLogos(QString tmdbId) = 0;
    virtual void concertClearArts(QString tmdbId) = 0;
    virtual void concertCdArts(QString tmdbId) = 0;
    virtual void tvShowImages(TvShow *show, QString tvdbId, QList<int> types) = 0;
    virtual void tvShowPosters(QString tvdbId) = 0;
    virtual void tvShowBackdrops(QString tvdbId) = 0;
    virtual void tvShowLogos(QString tvdbId) = 0;
    virtual void tvShowClearArts(QString tvdbId) = 0;
    virtual void tvShowCharacterArts(QString tvdbId) = 0;
    virtual void tvShowBanners(QString tvdbId) = 0;
    virtual void tvShowEpisodeThumb(QString tvdbId, int season, int episode) = 0;
    virtual void tvShowSeason(QString tvdbId, int season) = 0;
    virtual void tvShowSeasonBanners(QString tvdbId, int season) = 0;
    virtual void tvShowSeasonBackdrops(QString tvdbId, int season) = 0;
    virtual void tvShowSeasonThumbs(QString tvdbId, int season) = 0;
    virtual void tvShowThumbs(QString tvdbId) = 0;
    virtual void artistFanarts(QString mbId) = 0;
    virtual void artistLogos(QString mbId) = 0;
    virtual void artistThumbs(QString mbId) = 0;
    virtual void albumCdArts(QString mbId) = 0;
    virtual void albumThumbs(QString mbId) = 0;
    virtual void albumBooklets(QString mbId) = 0;
    virtual void artistImages(Artist *artist, QString mbId, QList<int> types) = 0;
    virtual void albumImages(Album *album, QString mbId, QList<int> types) = 0;
    virtual QList<int> provides() = 0;
    virtual bool hasSettings() = 0;
    virtual void loadSettings(QSettings &settings) = 0;
    virtual void saveSettings(QSettings &settings) = 0;
    virtual QWidget *settingsWidget() = 0;
    virtual QUrl siteUrl() = 0;

public slots:
    virtual void searchMovie(QString searchStr, int limit) = 0;
    virtual void searchConcert(QString searchStr, int limit) = 0;
    virtual void searchTvShow(QString searchStr, int limit) = 0;
    virtual void searchArtist(QString searchStr, int limit) = 0;
    virtual void searchAlbum(QString artistName, QString searchStr, int limit) = 0;
signals:
    virtual void sigSearchDone(QList<ScraperSearchResult>) = 0;
    virtual void sigImagesLoaded(QList<Poster>) = 0;
    virtual void sigImagesLoaded(Movie *, QMap<int, QList<Poster> >) = 0;
    virtual void sigImagesLoaded(Concert *, QMap<int, QList<Poster> >) = 0;
    virtual void sigImagesLoaded(TvShow *, QMap<int, QList<Poster> >) = 0;
    virtual void sigImagesLoaded(Artist *, QMap<int, QList<Poster> >) = 0;
    virtual void sigImagesLoaded(Album *, QMap<int, QList<Poster> >) = 0;
};

Q_DECLARE_METATYPE(ImageProviderInterface*)
Q_DECLARE_OPAQUE_POINTER(ImageProviderInterface*)

#endif // IMAGEPROVIDERINTERFACE_H
