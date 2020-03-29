#pragma once

#include "globals/Globals.h"
#include "tv_shows/TvDbId.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QVector>

class Album;
class Artist;
class Concert;
class Movie;
class TvShow;
class TvShowEpisode;

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject* parent = nullptr);
    ~Database() override;
    QSqlDatabase db();
    void transaction();
    void commit();
    void clearAllMovies();
    void clearMoviesInDirectory(QDir path);
    void add(Movie* movie, QDir path);
    void update(Movie* movie);
    QVector<Movie*> moviesInDirectory(QDir path);

    void clearAllConcerts();
    void clearConcertsInDirectory(QDir path);
    void add(Concert* concert, QDir path);
    void update(Concert* concert);
    QVector<Concert*> concertsInDirectory(QDir path);

    void add(TvShow* show, QDir path);
    void add(TvShowEpisode* episode, QDir path, int idShow);
    void update(TvShow* show);
    void update(TvShowEpisode* episode);
    void clearAllTvShows();
    void clearTvShowsInDirectory(QDir path);
    void clearTvShowInDirectory(QDir path);
    int showCount(QDir path);
    QVector<TvShow*> showsInDirectory(QDir path);
    QVector<TvShowEpisode*> episodes(int idShow);
    int episodeCount();

    void setShowMissingEpisodes(TvShow* show, bool showMissing);
    void setHideSpecialsInMissingEpisodes(TvShow* show, bool hideSpecials);
    int showsSettingsId(TvShow* show);
    void clearEpisodeList(int showsSettingsId);
    void cleanUpEpisodeList(int showsSettingsId);
    void addEpisodeToShowList(TvShowEpisode* episode, int showsSettingsId, TvDbId tvdbid);
    QVector<TvShowEpisode*> showsEpisodes(TvShow* show);

    void clearAllArtists();
    void clearArtistsInDirectory(QDir path);
    void add(Artist* artist, QDir path);
    void update(Artist* artist);
    QVector<Artist*> artistsInDirectory(QDir path);

    void clearAllAlbums();
    void clearAlbumsInDirectory(QDir path);
    void add(Album* album, QDir path);
    void update(Album* album);
    QVector<Album*> albums(Artist* artist);

    void addImport(QString fileName, QString type, QDir path);
    bool guessImport(QString fileName, QString& type, QString& path);

    void setLabel(QStringList fileNames, ColorLabel color);
    ColorLabel getLabel(QStringList fileNames);

private:
    QSqlDatabase* m_db;
    void updateDbVersion(int version);
};
