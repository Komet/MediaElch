#ifndef XBMCXML_H
#define XBMCXML_H

#include <QObject>
#include <QXmlStreamWriter>

#include "data/MediaCenterInterface.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

class XbmcXml : public MediaCenterInterface
{
    Q_OBJECT
public:
    explicit XbmcXml(QObject *parent = 0);
    ~XbmcXml();

    bool saveMovie(Movie *movie);
    bool loadMovie(Movie *movie);
    void loadMovieImages(Movie *movie);
    void exportDatabase(QList<Movie *> movies, QList<TvShow*> shows, QString exportPath, QString pathSearch, QString pathReplace);
    bool loadTvShow(TvShow *show);
    void loadTvShowImages(TvShow *show);
    bool loadTvShowEpisode(TvShowEpisode *episode);
    void loadTvShowEpisodeImages(TvShowEpisode *episode);
    bool saveTvShow(TvShow *show);
    bool saveTvShowEpisode(TvShowEpisode *episode);
signals:
    void sigExportStarted();
    void sigExportProgress(int, int);
    void sigExportDone();
    void sigExportRaiseError(QString);
private:
    void writeMovieXml(QXmlStreamWriter &xml, Movie *movie, bool writePath = false, QString pathSearch = "", QString pathReplace = "");
    void writeTvShowXml(QXmlStreamWriter &xml, TvShow *show, bool writePath = false, QString pathSearch = "", QString pathReplace = "", bool writeStartAndEndElement = true);
    void writeTvShowEpisodeXml(QXmlStreamWriter &xml, TvShowEpisode *episode, bool writePath = false, QString pathSearch = "", QString pathReplace = "");
};

#endif // XBMCXML_H
