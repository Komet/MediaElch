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
    void clearMovies(QString path = "");
    void add(Movie* movie, QString path);
    void update(Movie* movie);
    QVector<Movie*> movies(QString path);

    void clearConcerts(QString path = "");
    void add(Concert* concert, QString path);
    void update(Concert* concert);
    QVector<Concert*> concerts(QString path);

    void add(TvShow* show, QString path);
    void add(TvShowEpisode* episode, QString path, int idShow);
    void update(TvShow* show);
    void update(TvShowEpisode* episode);
    void clearTvShows(QString path = "");
    void clearTvShow(QString showDir);
    QVector<TvShow*> shows(QString path);
    QVector<TvShowEpisode*> episodes(int idShow);
    int episodeCount();

    void setShowMissingEpisodes(TvShow* show, bool showMissing);
    void setHideSpecialsInMissingEpisodes(TvShow* show, bool hideSpecials);
    int showsSettingsId(TvShow* show);
    void clearEpisodeList(int showsSettingsId);
    void cleanUpEpisodeList(int showsSettingsId);
    void addEpisodeToShowList(TvShowEpisode* episode, int showsSettingsId, TvDbId tvdbid);
    QVector<TvShowEpisode*> showsEpisodes(TvShow* show);

    void clearArtists(QString path = "");
    void add(Artist* artist, QString path);
    void update(Artist* artist);
    QVector<Artist*> artists(QString path);

    void clearAlbums(QString path = "");
    void add(Album* album, QString path);
    void update(Album* album);
    QVector<Album*> albums(Artist* artist);

    void addImport(QString fileName, QString type, QString path);
    bool guessImport(QString fileName, QString& type, QString& path);

    void setLabel(QStringList fileNames, ColorLabel color);
    ColorLabel getLabel(QStringList fileNames);

private:
    QSqlDatabase* m_db;
    void updateDbVersion(int version);
};
