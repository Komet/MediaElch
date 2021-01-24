#pragma once

#include "concerts/Concert.h"
#include "media_centers/KodiVersion.h"
#include "media_centers/MediaCenterInterface.h"
#include "music/Album.h"
#include "music/Artist.h"

#include <QByteArray>
#include <QDomDocument>
#include <QObject>
#include <QString>
#include <QVector>
#include <QXmlStreamWriter>

class Movie;
class TvShow;
class TvShowEpisode;
class Subtitle;

class KodiXml : public MediaCenterInterface
{
    Q_OBJECT
public:
    explicit KodiXml(QObject* parent = nullptr);
    ~KodiXml() override;

    void setVersion(mediaelch::KodiVersion version);

    // movies
    bool saveMovie(Movie* movie) override;
    bool loadMovie(Movie* movie, QString initialNfoContent = "") override;
    // movie images (e.g. posters)
    QImage movieSetPoster(QString setName) override;
    QImage movieSetBackdrop(QString setName) override;
    void saveMovieSetPoster(QString setName, QImage poster) override;
    void saveMovieSetBackdrop(QString setName, QImage backdrop) override;

    // concerts
    bool saveConcert(Concert* concert) override;
    bool loadConcert(Concert* concert, QString initialNfoContent = "") override;
    void loadConcertImages(Concert* concert);

    // TV shows
    bool loadTvShow(TvShow* show, QString initialNfoContent = "") override;
    bool loadTvShowEpisode(TvShowEpisode* episode, QString initialNfoContent = "") override;
    bool saveTvShow(TvShow* show) override;
    bool saveTvShowEpisode(TvShowEpisode* episode) override;

    // fanart
    QStringList extraFanartNames(Movie* movie) override;
    QStringList extraFanartNames(Concert* concert) override;
    QStringList extraFanartNames(TvShow* show) override;
    QStringList extraFanartNames(Artist* artist) override;

    // music
    bool saveArtist(Artist* artist) override;
    bool saveAlbum(Album* album) override;
    bool loadArtist(Artist* artist, QString initialNfoContent = "") override;
    bool loadAlbum(Album* album, QString initialNfoContent = "") override;

    // actors
    QString actorImageName(Movie* movie, Actor actor) override;
    QString actorImageName(TvShow* show, Actor actor) override;
    QString actorImageName(TvShowEpisode* episode, Actor actor) override;

    QString imageFileName(const Movie* movie,
        ImageType type,
        QVector<DataFile> dataFiles = QVector<DataFile>(),
        bool constructName = false) override;
    QString imageFileName(const Concert* concert,
        ImageType type,
        QVector<DataFile> dataFiles = QVector<DataFile>(),
        bool constructName = false) override;
    QString imageFileName(const TvShowEpisode* episode,
        ImageType type,
        QVector<DataFile> dataFiles = QVector<DataFile>(),
        bool constructName = false) override;
    QString imageFileName(const TvShow* show,
        ImageType type,
        SeasonNumber season = SeasonNumber::NoSeason,
        QVector<DataFile> dataFiles = QVector<DataFile>(),
        bool constructName = false) override;
    QString imageFileName(const Artist* artist,
        ImageType type,
        QVector<DataFile> dataFiles = QVector<DataFile>(),
        bool constructName = false) override;
    QString imageFileName(const Album* album,
        ImageType type,
        QVector<DataFile> dataFiles = QVector<DataFile>(),
        bool constructName = false) override;

    QString nfoFilePath(Movie* movie) override;
    QString nfoFilePath(Concert* concert) override;
    QString nfoFilePath(TvShowEpisode* episode) override;
    QString nfoFilePath(TvShow* show) override;
    QString nfoFilePath(Artist* artist) override;
    QString nfoFilePath(Album* album) override;

    static void writeStreamDetails(QXmlStreamWriter& xml,
        StreamDetails* streamDetails,
        const QVector<Subtitle*>& subtitles,
        bool hasStreamDetails);

    static void writeStringsAsOneTagEach(QXmlStreamWriter& xml, const QString& name, const QStringList& list);

    void loadBooklets(Album* album) override;

private:
    QByteArray getMovieXml(Movie* movie);
    QByteArray getConcertXml(Concert* concert);
    QByteArray getTvShowXml(TvShow* show);
    QByteArray getEpisodeXml(const QVector<TvShowEpisode*>& episodes);
    QByteArray getArtistXml(Artist* artist);
    QByteArray getAlbumXml(Album* album);
    bool loadStreamDetails(StreamDetails* streamDetails, QDomDocument domDoc);
    void loadStreamDetails(StreamDetails* streamDetails, QDomElement elem);
    bool saveFile(QString filename, QByteArray data);
    mediaelch::DirectoryPath getPath(const Movie* movie);
    mediaelch::DirectoryPath getPath(const Concert* concert);
    QString movieSetFileName(QString setName, DataFile* dataFile);

private:
    mediaelch::KodiVersion m_version;
};
