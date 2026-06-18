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
    virtual ~TvShowPersistence() = default;
    QSqlDatabase db();

    virtual void add(TvShow* show, mediaelch::DirectoryPath path);
    virtual void add(TvShowEpisode* episode, mediaelch::DirectoryPath path, mediaelch::DatabaseId idShow);
    virtual void update(TvShow* show);
    virtual void update(TvShowEpisode* episode);
    virtual void clearAllTvShows();
    virtual void clearTvShowsInDirectory(mediaelch::DirectoryPath path);
    virtual void clearTvShowInDirectory(mediaelch::DirectoryPath path);
    virtual int showCount(mediaelch::DirectoryPath path);
    virtual QVector<TvShow*> showsInDirectory(mediaelch::DirectoryPath path);
    virtual QVector<TvShowEpisode*> episodes(mediaelch::DatabaseId idShow);
    virtual int episodeCount();

    virtual void setShowMissingEpisodes(TvShow* show, bool showMissing);
    virtual void setHideSpecialsInMissingEpisodes(TvShow* show, bool hideSpecials);
    virtual mediaelch::DatabaseId showsSettingsId(TvShow* show);
    virtual void clearEpisodeList(mediaelch::DatabaseId showsSettingsId);
    virtual void cleanUpEpisodeList(mediaelch::DatabaseId showsSettingsId);
    virtual void addEpisodeToShowList(TvShowEpisode* episode, mediaelch::DatabaseId showsSettingsId, TmdbId tmdbId);
    virtual QVector<TvShowEpisode*> showsEpisodes(TvShow* show);

private:
    Database& m_db;
};

} // namespace mediaelch
