#ifndef XBMCXML_H
#define XBMCXML_H

#include <QDomDocument>
#include <QObject>
#include <QXmlStreamWriter>

#include "data/Concert.h"
#include "data/MediaCenterInterface.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

/**
 * @brief The XbmcXml class
 */
class XbmcXml : public MediaCenterInterface
{
    Q_OBJECT
public:
    explicit XbmcXml(QObject *parent = 0);
    ~XbmcXml();

    bool saveMovie(Movie *movie);
    bool loadMovie(Movie *movie, QString initialNfoContent = "");
    bool saveConcert(Concert *concert);
    bool loadConcert(Concert *concert, QString initialNfoContent = "");
    void loadConcertImages(Concert *concert);
    bool loadTvShow(TvShow *show, QString initialNfoContent = "");
    bool loadTvShowEpisode(TvShowEpisode *episode, QString initialNfoContent = "");
    bool saveTvShow(TvShow *show);
    bool saveTvShowEpisode(TvShowEpisode *episode);
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
    static void saveAdditionalImages(Movie *movie);
    static void saveAdditionalImages(Concert *concert);
    static void saveAdditionalImages(TvShow *show);
    static QString logoImageNameStatic(Movie *movie);
    static QString clearArtImageNameStatic(Movie *movie);
    static QString cdArtImageNameStatic(Movie *movie);
    static QString logoImageNameStatic(Concert *concert);
    static QString clearArtImageNameStatic(Concert *concert);
    static QString cdArtImageNameStatic(Concert *concert);
    static QString logoImageNameStatic(TvShow *show);
    static QString clearArtImageNameStatic(TvShow *show);
    static QString characterArtImageNameStatic(TvShow *show);
    QString logoImageName(TvShow *show);
    QString clearArtImageName(TvShow *show);
    QString characterArtImageName(TvShow *show);

private:
    void writeMovieXml(QXmlStreamWriter &xml, Movie *movie);
    void writeConcertXml(QXmlStreamWriter &xml, Concert *concert);
    void writeTvShowXml(QXmlStreamWriter &xml, TvShow *show);
    void writeTvShowEpisodeXml(QXmlStreamWriter &xml, TvShowEpisode *episode);
    void writeStreamDetails(QXmlStreamWriter &xml, StreamDetails *streamDetails);
    QString nfoFilePath(Movie *movie);
    QString nfoFilePath(Concert *concert);
    bool loadStreamDetails(StreamDetails *streamDetails, QDomDocument domDoc);
};

#endif // XBMCXML_H
