#ifndef XBMCXML_H
#define XBMCXML_H

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
    bool loadMovie(Movie *movie);
    bool saveConcert(Concert *concert);
    bool loadConcert(Concert *concert);
    void loadConcertImages(Concert *concert);
    void exportDatabase(QList<Movie *> movies, QList<TvShow*> shows, QString exportPath, QString pathSearch, QString pathReplace);
    bool loadTvShow(TvShow *show);
    bool loadTvShowEpisode(TvShowEpisode *episode);
    bool saveTvShow(TvShow *show);
    bool saveTvShowEpisode(TvShowEpisode *episode);
    void shutdown();
    bool hasFeature(int feature);
    QImage movieSetPoster(QString setName);
    QImage movieSetBackdrop(QString setName);
    void saveMovieSetPoster(QString setName, QImage poster);
    void saveMovieSetBackdrop(QString setName, QImage backdrop);
    QString posterImageName(Movie *movie);
    QString backdropImageName(Movie *movie);
    QString actorImageName(Movie *movie, Actor actor);
    QString posterImageName(Concert *concert);
    QString backdropImageName(Concert *concert);
    QString thumbnailImageName(TvShowEpisode *episode);
    QString posterImageName(TvShow *show);
    QString backdropImageName(TvShow *show);
    QString bannerImageName(TvShow *show);
    QString actorImageName(TvShow *show, Actor actor);
    QString seasonPosterImageName(TvShow *show, int season);

signals:
    void sigExportStarted();
    void sigExportProgress(int, int);
    void sigExportDone();
    void sigExportRaiseError(QString);

private:
    void writeMovieXml(QXmlStreamWriter &xml, Movie *movie, bool writePath = false, QString pathSearch = "", QString pathReplace = "");
    void writeConcertXml(QXmlStreamWriter &xml, Concert *concert, bool writePath = false, QString pathSearch = "", QString pathReplace = "");
    void writeTvShowXml(QXmlStreamWriter &xml, TvShow *show, bool writePath = false, QString pathSearch = "", QString pathReplace = "", bool writeStartAndEndElement = true);
    void writeTvShowEpisodeXml(QXmlStreamWriter &xml, TvShowEpisode *episode, bool writePath = false, QString pathSearch = "", QString pathReplace = "");
};

#endif // XBMCXML_H
