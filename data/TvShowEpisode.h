#ifndef TVSHOWEPISODE_H
#define TVSHOWEPISODE_H

#include <QMetaType>
#include <QObject>
#include <QStringList>
#include "data/MediaCenterInterface.h"
#include "data/StreamDetails.h"
#include "data/TvScraperInterface.h"
#include "data/TvShow.h"
#include "data/TvShowModelItem.h"

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
    Q_PROPERTY(qreal ratin READ rating WRITE setRating)
    Q_PROPERTY(int season READ season WRITE setSeason)
    Q_PROPERTY(int episode READ episode WRITE setEpisode)
    Q_PROPERTY(int displaySeason READ displaySeason WRITE setDisplaySeason)
    Q_PROPERTY(int displayEpisode READ displayEpisode WRITE setDisplayEpisode)
    Q_PROPERTY(QString overview READ overview WRITE setOverview)
    Q_PROPERTY(QStringList writers READ writers WRITE setWriters)
    Q_PROPERTY(QStringList directors READ directors WRITE setDirectors)
    Q_PROPERTY(int playCount READ playCount WRITE setPlayCount)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed WRITE setLastPlayed)
    Q_PROPERTY(QDate firstAired READ firstAired WRITE setFirstAired)
    Q_PROPERTY(QString certification READ certification WRITE setCertification)
    Q_PROPERTY(QString network READ network WRITE setNetwork)

public:
    explicit TvShowEpisode(QStringList files = QStringList(), TvShow *parent = 0);
    void clear();
    void clear(QList<int> infos);

    void setFiles(QStringList files);
    virtual TvShow *tvShow();
    virtual QStringList files() const;
    virtual QString showTitle() const;
    virtual QString name() const;
    virtual QString completeEpisodeName() const;
    virtual qreal rating() const;
    virtual int votes() const;
    virtual int top250() const;
    virtual int season() const;
    virtual int episode() const;
    virtual int displaySeason() const;
    virtual int displayEpisode() const;
    virtual QString overview() const;
    virtual QStringList writers() const;
    virtual QStringList directors() const;
    virtual int playCount() const;
    virtual QDateTime lastPlayed() const;
    virtual QDate firstAired() const;
    virtual QTime epBookmark() const;
    virtual QString certification() const;
    virtual QString network() const;
    virtual QString seasonString() const;
    virtual QString episodeString() const;
    virtual bool isValid() const;
    virtual QUrl thumbnail() const;
    virtual QByteArray thumbnailImage();
    virtual bool thumbnailImageChanged() const;
    virtual TvShowModelItem *modelItem();
    virtual bool hasChanged() const;
    virtual QList<QString*> writersPointer();
    virtual QList<QString*> directorsPointer();
    virtual bool infoLoaded() const;
    virtual int episodeId() const;
    virtual StreamDetails *streamDetails();
    virtual bool streamDetailsLoaded() const;
    virtual QString nfoContent() const;
    virtual int databaseId() const;
    virtual bool syncNeeded() const;
    virtual bool isDummy() const;

    void setShow(TvShow *show);
    void setName(QString name);
    void setShowTitle(QString showTitle);
    void setRating(qreal rating);
    void setVotes(int votes);
    void setTop250(int top250);
    void setSeason(int season);
    void setEpisode(int episode);
    void setDisplaySeason(int season);
    void setDisplayEpisode(int episode);
    void setOverview(QString overview);
    void setWriters(QStringList writers);
    void addWriter(QString writer);
    void setDirectors(QStringList directors);
    void addDirector(QString director);
    void setPlayCount(int playCount);
    void setLastPlayed(QDateTime lastPlayed);
    void setFirstAired(QDate firstAired);
    void setCertification(QString certification);
    void setNetwork(QString network);
    void setThumbnail(QUrl url);
    void setThumbnailImage(QByteArray thumbnail);
    void setEpBookmark(QTime epBookmark);
    void setInfosLoaded(bool loaded);
    void setChanged(bool changed);
    void setModelItem(TvShowModelItem *item);
    void setStreamDetailsLoaded(bool loaded);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
    void setSyncNeeded(bool syncNeeded);
    void setIsDummy(bool dummy);

    void removeWriter(QString *writer);
    void removeDirector(QString *director);

    QList<Actor> actors() const;
    QList<Actor*> actorsPointer();
    void addActor(Actor actor);
    void removeActor(Actor *actor);

    bool loadData(MediaCenterInterface *mediaCenterInterface, bool reloadFromNfo = true);
    void loadData(QString id, TvScraperInterface *tvScraperInterface, QList<int> infosToLoad);
    bool saveData(MediaCenterInterface *mediaCenterInterface);
    void loadStreamDetailsFromFile();
    void clearImages();
    QList<int> infosToLoad();

    QList<int> imagesToRemove() const;
    void removeImage(int type);

    void scraperLoadDone();

    static bool lessThan(TvShowEpisode *a, TvShowEpisode *b);

    QString imdbId() const;
    void setImdbId(const QString &imdbId);

signals:
    void sigLoaded();
    void sigChanged(TvShowEpisode*);

private:
    QStringList m_files;
    TvShow *m_parent;
    QString m_name;
    QString m_showTitle;
    qreal m_rating;
    QString m_imdbId;
    int m_votes;
    int m_top250;
    int m_season;
    int m_episode;
    int m_displaySeason;
    int m_displayEpisode;
    QString m_overview;
    QStringList m_writers;
    QStringList m_directors;
    int m_playCount;
    QDateTime m_lastPlayed;
    QDate m_firstAired;
    QTime m_epBookmark;
    QString m_certification;
    QString m_network;
    QUrl m_thumbnail;
    QByteArray m_thumbnailImage;
    TvShowModelItem *m_modelItem;
    bool m_thumbnailImageChanged;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    bool m_hasChanged;
    int m_episodeId;
    bool m_streamDetailsLoaded;
    StreamDetails *m_streamDetails;
    QString m_nfoContent;
    int m_databaseId;
    bool m_syncNeeded;
    QList<int> m_infosToLoad;
    QList<int> m_imagesToRemove;
    bool m_isDummy;
    QList<Actor> m_actors;
};

QDebug operator<<(QDebug dbg, const TvShowEpisode &episode);
QDebug operator<<(QDebug dbg, const TvShowEpisode *episode);

Q_DECLARE_METATYPE(TvShowEpisode*)

#endif // TVSHOWEPISODE_H
