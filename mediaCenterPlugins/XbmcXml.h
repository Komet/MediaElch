#ifndef XBMCXML_H
#define XBMCXML_H

#include <QDomDocument>
#include <QObject>
#include <QXmlStreamWriter>

#include "data/Concert.h"
#include "data/MediaCenterInterface.h"
#include "movies/Movie.h"
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
    QStringList extraFanartNames(Movie *movie);
    QStringList extraFanartNames(Concert *concert);
    QStringList extraFanartNames(TvShow *show);
    QImage movieSetPoster(QString setName);
    QImage movieSetBackdrop(QString setName);
    void saveMovieSetPoster(QString setName, QImage poster);
    void saveMovieSetBackdrop(QString setName, QImage backdrop);
    QString posterImageName(Movie *movie, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString backdropImageName(Movie *movie, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString logoImageName(Movie *movie, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString clearArtImageName(Movie *movie, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString cdArtImageName(Movie *movie, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString actorImageName(Movie *movie, Actor actor);
    QString posterImageName(Concert *concert, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString backdropImageName(Concert *concert, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString logoImageName(Concert *concert, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString clearArtImageName(Concert *concert, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString cdArtImageName(Concert *concert, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString thumbnailImageName(TvShowEpisode *episode, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString posterImageName(TvShow *show, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString backdropImageName(TvShow *show, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString bannerImageName(TvShow *show, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString actorImageName(TvShow *show, Actor actor);
    QString seasonPosterImageName(TvShow *show, int season, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString seasonBackdropImageName(TvShow *show, int season, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString seasonBannerImageName(TvShow *show, int season, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    void saveAdditionalImages(Movie *movie);
    void saveAdditionalImages(Concert *concert);
    void saveAdditionalImages(TvShow *show);
    QString logoImageName(TvShow *show, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString clearArtImageName(TvShow *show, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString characterArtImageName(TvShow *show, QList<DataFile> dataFiles = QList<DataFile>(), bool constructName = false);
    QString nfoFilePath(Movie *movie);
    QString nfoFilePath(Concert *concert);
    QString nfoFilePath(TvShowEpisode *episode);

private:
    void writeMovieXml(QXmlStreamWriter &xml, Movie *movie);
    void writeConcertXml(QXmlStreamWriter &xml, Concert *concert);
    void writeTvShowXml(QXmlStreamWriter &xml, TvShow *show);
    void writeTvShowEpisodeXml(QXmlStreamWriter &xml, TvShowEpisode *episode);
    void writeStreamDetails(QXmlStreamWriter &xml, StreamDetails *streamDetails);
    bool loadStreamDetails(StreamDetails *streamDetails, QDomDocument domDoc);
    void loadStreamDetails(StreamDetails *streamDetails, QDomElement elem);
    bool saveFile(QString filename, QByteArray data);
    QString getPath(Movie *movie);
    QString getPath(Concert *concert);
    QString movieSetFileName(QString setName, QString name);
};

#endif // XBMCXML_H
