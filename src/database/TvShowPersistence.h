#pragma once

#include "data/TmdbId.h"
#include "database/DatabaseId.h"
#include "media/Path.h"

#include <QObject>
#include <QSqlDatabase>

class TvShow;
class TvShowEpisode;
class Database;

namespace mediaelch {

class TvShowPersistence
{
public:
    explicit TvShowPersistence(Database& db);
    QSqlDatabase db();


    void add(TvShow* show, mediaelch::DirectoryPath path);
    void add(TvShowEpisode* episode, mediaelch::DirectoryPath path, mediaelch::DatabaseId idShow);
    void update(TvShow* show);
    void update(TvShowEpisode* episode);
    void clearAllTvShows();
    void clearTvShowsInDirectory(mediaelch::DirectoryPath path);
    void clearTvShowInDirectory(mediaelch::DirectoryPath path);
    int showCount(mediaelch::DirectoryPath path);
    QVector<TvShow*> showsInDirectory(mediaelch::DirectoryPath path);
    QVector<TvShowEpisode*> episodes(mediaelch::DatabaseId idShow);
    int episodeCount();

    void setShowMissingEpisodes(TvShow* show, bool showMissing);
    void setHideSpecialsInMissingEpisodes(TvShow* show, bool hideSpecials);
    mediaelch::DatabaseId showsSettingsId(TvShow* show);
    void clearEpisodeList(mediaelch::DatabaseId showsSettingsId);
    void cleanUpEpisodeList(mediaelch::DatabaseId showsSettingsId);
    void addEpisodeToShowList(TvShowEpisode* episode, mediaelch::DatabaseId showsSettingsId, TmdbId tmdbId);
    QVector<TvShowEpisode*> showsEpisodes(TvShow* show);


private:
    Database& m_db;
};

} // namespace mediaelch
