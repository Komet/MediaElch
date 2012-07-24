#ifndef TVSHOWEPISODE_H
#define TVSHOWEPISODE_H

#include <QMetaType>
#include <QObject>
#include <QStringList>
#include "data/MediaCenterInterface.h"
#include "data/TvScraperInterface.h"
#include "data/TvShow.h"
#include "data/TvShowModelItem.h"

class TvShow;
class MediaCenterInterface;
class TvScraperInterface;
class TvShowModelItem;

class TvShowEpisode : public QObject
{
    Q_OBJECT
public:
    explicit TvShowEpisode(QStringList files = QStringList(), TvShow *parent = 0);
    void moveToMainThread();
    void clear();

    TvShow *tvShow();
    QStringList files() const;
    QString showTitle() const;
    QString name() const;
    QString completeEpisodeName() const;
    qreal rating() const;
    int season() const;
    int episode() const;
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
    QImage *thumbnailImage();
    bool thumbnailImageChanged() const;
    TvShowModelItem *modelItem();
    bool hasChanged() const;
    QList<QString*> writersPointer();
    QList<QString*> directorsPointer();
    bool infoLoaded() const;
    int episodeId() const;

    void setName(QString name);
    void setShowTitle(QString showTitle);
    void setRating(qreal rating);
    void setSeason(int season);
    void setEpisode(int episode);
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
    void setThumbnailImage(QImage thumbnail);
    void setInfosLoaded(bool loaded);
    void setChanged(bool changed);
    void setModelItem(TvShowModelItem *item);

    void removeWriter(QString *writer);
    void removeDirector(QString *director);

    bool loadData(MediaCenterInterface *mediaCenterInterface);
    void loadData(QString id, TvScraperInterface *tvScraperInterface);
    bool saveData(MediaCenterInterface *mediaCenterInterface);
    void loadImages(MediaCenterInterface *mediaCenterInterface);

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
    QString m_overview;
    QStringList m_writers;
    QStringList m_directors;
    int m_playCount;
    QDateTime m_lastPlayed;
    QDate m_firstAired;
    QString m_certification;
    QString m_network;
    QUrl m_thumbnail;
    QImage m_thumbnailImage;
    TvShowModelItem *m_modelItem;
    bool m_thumbnailImageChanged;
    bool m_infoLoaded;
    bool m_hasChanged;
    int m_episodeId;
};

QDebug operator<<(QDebug dbg, const TvShowEpisode &episode);
QDebug operator<<(QDebug dbg, const TvShowEpisode *episode);

Q_DECLARE_METATYPE(TvShowEpisode*)

#endif // TVSHOWEPISODE_H
