#pragma once

#include "data/Certification.h"
#include "data/EpisodeNumber.h"
#include "data/ImdbId.h"
#include "data/Rating.h"
#include "data/SeasonNumber.h"
#include "data/StreamDetails.h"
#include "data/TvDbId.h"
#include "globals/Globals.h"
#include "tvShows/TvShowModelItem.h"

#include <QMetaType>
#include <QObject>
#include <QStringList>

class MediaCenterInterface;
class StreamDetails;
class TvScraperInterface;
class TvShow;
class TvShowModelItem;

/**
 * @brief The TvShowEpisode class
 */
class TvShowEpisode final : public QObject
{
    Q_OBJECT

public:
    explicit TvShowEpisode(QStringList files = QStringList(), TvShow* parent = nullptr);
    void clear();
    void clear(QVector<TvShowScraperInfos> infos);

    void setFiles(QStringList files);
    TvShow* tvShow() const;
    QStringList files() const;
    QString showTitle() const;
    QString name() const;
    QString completeEpisodeName() const;
    double rating() const;
    int votes() const;
    int top250() const;
    SeasonNumber season() const;
    EpisodeNumber episode() const;
    SeasonNumber displaySeason() const;
    EpisodeNumber displayEpisode() const;
    QString overview() const;
    QStringList writers() const;
    QStringList directors() const;
    int playCount() const;
    QDateTime lastPlayed() const;
    QDate firstAired() const;
    QTime epBookmark() const;
    Certification certification() const;
    QString network() const;
    QString seasonString() const;
    QString episodeString() const;
    bool isValid() const;
    QUrl thumbnail() const;
    QByteArray thumbnailImage();
    bool thumbnailImageChanged() const;
    TvShowModelItem* modelItem();
    bool hasChanged() const;
    QVector<QString*> writersPointer();
    QVector<QString*> directorsPointer();
    bool infoLoaded() const;
    int episodeId() const;
    StreamDetails* streamDetails();
    bool streamDetailsLoaded() const;
    QString nfoContent() const;
    int databaseId() const;
    bool syncNeeded() const;
    bool isDummy() const;

    void setShow(TvShow* show);
    void setName(QString name);
    void setShowTitle(QString showTitle);
    void setRating(double rating);
    void setVotes(int votes);
    void setTop250(int top250);
    void setSeason(SeasonNumber season);
    void setEpisode(EpisodeNumber episode);
    void setDisplaySeason(SeasonNumber season);
    void setDisplayEpisode(EpisodeNumber episode);
    void setOverview(QString overview);
    void setWriters(QStringList writers);
    void addWriter(QString writer);
    void setDirectors(QStringList directors);
    void addDirector(QString director);
    void setPlayCount(int playCount);
    void setLastPlayed(QDateTime lastPlayed);
    void setFirstAired(QDate firstAired);
    void setCertification(Certification certification);
    void setNetwork(QString network);
    void setThumbnail(QUrl url);
    void setThumbnailImage(QByteArray thumbnail);
    void setEpBookmark(QTime epBookmark);
    void setInfosLoaded(bool loaded);
    void setChanged(bool changed);
    void setModelItem(TvShowModelItem* item);
    void setStreamDetailsLoaded(bool loaded);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
    void setSyncNeeded(bool syncNeeded);
    void setIsDummy(bool dummy);

    void removeWriter(QString* writer);
    void removeDirector(QString* director);

    QVector<Actor> actors() const;
    QVector<Actor*> actorsPointer();
    void addActor(Actor actor);
    void removeActor(Actor* actor);

    bool loadData(MediaCenterInterface* mediaCenterInterface, bool reloadFromNfo = true);
    void loadData(TvDbId id, TvScraperInterface* tvScraperInterface, QVector<TvShowScraperInfos> infosToLoad);
    bool saveData(MediaCenterInterface* mediaCenterInterface);
    void loadStreamDetailsFromFile();
    void clearImages();
    QVector<TvShowScraperInfos> infosToLoad();

    QVector<ImageType> imagesToRemove() const;
    void removeImage(ImageType type);

    void scraperLoadDone();

    static bool lessThan(TvShowEpisode* a, TvShowEpisode* b);

    ImdbId imdbId() const;
    void setImdbId(const ImdbId& imdbId);
    TvDbId tvdbId() const;
    void setTvdbId(const TvDbId& tvdbId);

signals:
    void sigLoaded();
    void sigChanged(TvShowEpisode*);

private:
    QStringList m_files;
    TvShow* m_parent;
    QString m_name;
    QString m_showTitle;
    Rating m_rating;
    ImdbId m_imdbId;
    TvDbId m_tvdbId;
    SeasonNumber m_season;
    EpisodeNumber m_episode;
    SeasonNumber m_displaySeason;
    EpisodeNumber m_displayEpisode;
    QString m_overview;
    QStringList m_writers;
    QStringList m_directors;
    int m_playCount;
    QDateTime m_lastPlayed;
    QDate m_firstAired;
    QTime m_epBookmark;
    Certification m_certification;
    QString m_network;
    QUrl m_thumbnail;
    QByteArray m_thumbnailImage;
    TvShowModelItem* m_modelItem;
    bool m_thumbnailImageChanged;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    bool m_hasChanged;
    int m_episodeId;
    bool m_streamDetailsLoaded;
    StreamDetails* m_streamDetails;
    QString m_nfoContent;
    int m_databaseId;
    bool m_syncNeeded;
    QVector<TvShowScraperInfos> m_infosToLoad;
    QVector<ImageType> m_imagesToRemove;
    bool m_isDummy;
    QVector<Actor> m_actors;
};

QDebug operator<<(QDebug dbg, const TvShowEpisode& episode);
QDebug operator<<(QDebug dbg, const TvShowEpisode* episode);

Q_DECLARE_METATYPE(TvShowEpisode*)
