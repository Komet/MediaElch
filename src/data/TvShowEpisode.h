#pragma once

#include "data/Certification.h"
#include "data/EpisodeNumber.h"
#include "data/ImdbId.h"
#include "data/Rating.h"
#include "data/SeasonNumber.h"
#include "data/StreamDetails.h"
#include "data/TvDbId.h"
#include "data/TvShowModelItem.h"
#include "globals/Globals.h"

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
class TvShowEpisode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(SeasonNumber season READ season WRITE setSeason)
    Q_PROPERTY(EpisodeNumber episode READ episode WRITE setEpisode)
    Q_PROPERTY(SeasonNumber displaySeason READ displaySeason WRITE setDisplaySeason)
    Q_PROPERTY(EpisodeNumber displayEpisode READ displayEpisode WRITE setDisplayEpisode)
    Q_PROPERTY(QString overview READ overview WRITE setOverview)
    Q_PROPERTY(QStringList writers READ writers WRITE setWriters)
    Q_PROPERTY(QStringList directors READ directors WRITE setDirectors)
    Q_PROPERTY(int playCount READ playCount WRITE setPlayCount)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed WRITE setLastPlayed)
    Q_PROPERTY(QDate firstAired READ firstAired WRITE setFirstAired)
    Q_PROPERTY(Certification certification READ certification WRITE setCertification)
    Q_PROPERTY(QString network READ network WRITE setNetwork)

public:
    explicit TvShowEpisode(QStringList files = QStringList(), TvShow* parent = nullptr);
    void clear();
    void clear(QVector<TvShowScraperInfos> infos);

    void setFiles(QStringList files);
    virtual TvShow* tvShow() const;
    virtual QStringList files() const;
    virtual QString showTitle() const;
    virtual QString name() const;
    virtual QString completeEpisodeName() const;
    virtual double rating() const;
    virtual int votes() const;
    virtual int top250() const;
    virtual SeasonNumber season() const;
    virtual EpisodeNumber episode() const;
    virtual SeasonNumber displaySeason() const;
    virtual EpisodeNumber displayEpisode() const;
    virtual QString overview() const;
    virtual QStringList writers() const;
    virtual QStringList directors() const;
    virtual int playCount() const;
    virtual QDateTime lastPlayed() const;
    virtual QDate firstAired() const;
    virtual QTime epBookmark() const;
    virtual Certification certification() const;
    virtual QString network() const;
    virtual QString seasonString() const;
    virtual QString episodeString() const;
    virtual bool isValid() const;
    virtual QUrl thumbnail() const;
    virtual QByteArray thumbnailImage();
    virtual bool thumbnailImageChanged() const;
    virtual TvShowModelItem* modelItem();
    virtual bool hasChanged() const;
    virtual QVector<QString*> writersPointer();
    virtual QVector<QString*> directorsPointer();
    virtual bool infoLoaded() const;
    virtual int episodeId() const;
    virtual StreamDetails* streamDetails();
    virtual bool streamDetailsLoaded() const;
    virtual QString nfoContent() const;
    virtual int databaseId() const;
    virtual bool syncNeeded() const;
    virtual bool isDummy() const;

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
