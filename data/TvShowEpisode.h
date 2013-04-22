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

class TvShow;
class MediaCenterInterface;
class TvScraperInterface;
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

    TvShow *tvShow();
    QStringList files() const;
    QString showTitle() const;
    QString name() const;
    QString completeEpisodeName() const;
    qreal rating() const;
    int season() const;
    int episode() const;
    int displaySeason() const;
    int displayEpisode() const;
    QString overview() const;
    QStringList writers() const;
    QStringList directors() const;
    int playCount() const;
    QDateTime lastPlayed() const;
    QDate firstAired() const;
    QString certification() const;
    QString network() const;
    QString seasonString() const;
    QString episodeString() const;
    bool isValid() const;
    QUrl thumbnail() const;
    QByteArray thumbnailImage();
    bool thumbnailImageChanged() const;
    TvShowModelItem *modelItem();
    bool hasChanged() const;
    QList<QString*> writersPointer();
    QList<QString*> directorsPointer();
    bool infoLoaded() const;
    int episodeId() const;
    StreamDetails *streamDetails();
    bool streamDetailsLoaded() const;
    QString nfoContent() const;
    int databaseId() const;
    bool syncNeeded() const;

    void setShow(TvShow *show);
    void setName(QString name);
    void setShowTitle(QString showTitle);
    void setRating(qreal rating);
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
    void setInfosLoaded(bool loaded);
    void setChanged(bool changed);
    void setModelItem(TvShowModelItem *item);
    void setStreamDetailsLoaded(bool loaded);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
    void setSyncNeeded(bool syncNeeded);

    void removeWriter(QString *writer);
    void removeDirector(QString *director);

    bool loadData(MediaCenterInterface *mediaCenterInterface, bool reloadFromNfo = true);
    void loadData(QString id, TvScraperInterface *tvScraperInterface, QList<int> infosToLoad);
    bool saveData(MediaCenterInterface *mediaCenterInterface);
    void loadStreamDetailsFromFile();
    void clearImages();
    QList<int> infosToLoad();

    QList<ImageType> imagesToRemove() const;
    void removeImage(ImageType type);

    void scraperLoadDone();

signals:
    void sigLoaded();
    void sigChanged(TvShowEpisode*);

private:
    QStringList m_files;
    TvShow *m_parent;
    QString m_name;
    QString m_showTitle;
    qreal m_rating;
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
    QList<ImageType> m_imagesToRemove;
};

QDebug operator<<(QDebug dbg, const TvShowEpisode &episode);
QDebug operator<<(QDebug dbg, const TvShowEpisode *episode);

Q_DECLARE_METATYPE(TvShowEpisode*)

#endif // TVSHOWEPISODE_H
