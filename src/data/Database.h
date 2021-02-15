#pragma once

#include "file/Path.h"
#include "globals/Globals.h"
#include "tv_shows/TvDbId.h"

#include <QDateTime>
#include <QMutex>
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
    void clearMoviesInDirectory(mediaelch::DirectoryPath path);
    void add(Movie* movie, mediaelch::DirectoryPath path);
    void update(Movie* movie);
    QVector<Movie*> moviesInDirectory(mediaelch::DirectoryPath path);

    void clearAllConcerts();
    void clearConcertsInDirectory(mediaelch::DirectoryPath path);
    void add(Concert* concert, mediaelch::DirectoryPath path);
    void update(Concert* concert);
    QVector<Concert*> concertsInDirectory(mediaelch::DirectoryPath path);

    void add(TvShow* show, mediaelch::DirectoryPath path);
    void add(TvShowEpisode* episode, mediaelch::DirectoryPath path, int idShow);
    void update(TvShow* show);
    void update(TvShowEpisode* episode);
    void clearAllTvShows();
    void clearTvShowsInDirectory(mediaelch::DirectoryPath path);
    void clearTvShowInDirectory(mediaelch::DirectoryPath path);
    int showCount(mediaelch::DirectoryPath path);
    QVector<TvShow*> showsInDirectory(mediaelch::DirectoryPath path);
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
    void clearArtistsInDirectory(mediaelch::DirectoryPath path);
    void add(Artist* artist, mediaelch::DirectoryPath path);
    void update(Artist* artist);
    QVector<Artist*> artistsInDirectory(mediaelch::DirectoryPath path);

    void clearAllAlbums();
    void clearAlbumsInDirectory(mediaelch::DirectoryPath path);
    void add(Album* album, mediaelch::DirectoryPath path);
    void update(Album* album);
    QVector<Album*> albums(Artist* artist);

    void addImport(QString fileName, QString type, mediaelch::DirectoryPath path);
    bool guessImport(QString fileName, QString& type, QString& path);

    void setLabel(const mediaelch::FileList& fileNames, ColorLabel color);
    ColorLabel getLabel(const mediaelch::FileList& fileNames);

private:
    QMutex m_mutex;
    QSqlDatabase* m_db;
    void updateDbVersion(int version);
};
