#pragma once

#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Rating.h"
#include "data/StreamDetails.h"
#include "globals/Actor.h"
#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/TvDbId.h"

#include <QMetaType>
#include <QObject>
#include <QStringList>
#include <memory>
#include <vector>

class MediaCenterInterface;
class StreamDetails;
class TvScraperInterface;
class TvShow;
class EpisodeModelItem;

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
    QVector<Rating>& ratings();
    const QVector<Rating>& ratings() const;
    double userRating() const;
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
    EpisodeModelItem* modelItem();
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
    void setUserRating(double rating);
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
    void setModelItem(EpisodeModelItem* item);
    void setStreamDetailsLoaded(bool loaded);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
    void setSyncNeeded(bool syncNeeded);
    void setIsDummy(bool dummy);

    void removeWriter(QString* writer);
    void removeDirector(QString* director);

    QVector<const Actor*> actors() const;
    QVector<Actor*> actors();
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
    TvShow* m_parent = nullptr;
    QString m_name;
    QString m_showTitle;
    QVector<Rating> m_ratings;
    double m_userRating = 0.0;
    int m_imdbTop250 = 0;
    ImdbId m_imdbId;
    TvDbId m_tvdbId;
    SeasonNumber m_season;
    EpisodeNumber m_episode;
    SeasonNumber m_displaySeason;
    EpisodeNumber m_displayEpisode;
    QString m_overview;
    QStringList m_writers;
    QStringList m_directors;
    int m_playCount = 0;
    QDateTime m_lastPlayed;
    QDate m_firstAired;
    QTime m_epBookmark;
    Certification m_certification;
    QString m_network;
    QUrl m_thumbnail;
    QByteArray m_thumbnailImage;
    EpisodeModelItem* m_modelItem = nullptr;
    bool m_thumbnailImageChanged = false;
    bool m_infoLoaded = false;
    bool m_infoFromNfoLoaded = false;
    bool m_hasChanged = false;
    int m_episodeId = -1;
    bool m_streamDetailsLoaded = false;
    StreamDetails* m_streamDetails = nullptr;
    QString m_nfoContent;
    int m_databaseId = -1;
    bool m_syncNeeded = false;
    QVector<TvShowScraperInfos> m_infosToLoad;
    QVector<ImageType> m_imagesToRemove;
    bool m_isDummy = false;
    std::vector<std::unique_ptr<Actor>> m_actors;
};

QDebug operator<<(QDebug dbg, const TvShowEpisode& episode);
QDebug operator<<(QDebug dbg, const TvShowEpisode* episode);

Q_DECLARE_METATYPE(TvShowEpisode*)
