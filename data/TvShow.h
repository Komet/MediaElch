#ifndef TVSHOW_H
#define TVSHOW_H

#include <QMetaType>
#include <QObject>
#include <QStringList>
#include "data/MediaCenterInterface.h"
#include "data/TvScraperInterface.h"
#include "data/TvShowEpisode.h"

class TvShowEpisode;
class TvShowModelItem;
class TvScraperInterface;

class TvShow : public QObject
{
    Q_OBJECT
public:
    explicit TvShow(QString dir = QString(), QObject *parent = 0);
    void clear();
    void addEpisode(TvShowEpisode *episode);
    int episodeCount();
    void moveToMainThread();

    QString name() const;
    QString showTitle() const;
    QString dir() const;
    qreal rating() const;
    QDate firstAired() const;
    QStringList genres() const;
    QList<QString*> genresPointer();
    QString certification() const;
    QString network() const;
    QString overview() const;
    QStringList certifications() const;
    QList<Actor> actors() const;
    QList<Actor*> actorsPointer();
    QList<Poster> posters() const;
    QList<Poster> backdrops() const;
    QList<Poster> banners() const;
    QList<Poster> seasonPosters(int season) const;
    QImage *posterImage();
    QImage *backdropImage();
    QImage *bannerImage();
    QImage *seasonPosterImage(int season);
    bool posterImageChanged() const;
    bool backdropImageChanged() const;
    bool bannerImageChanged() const;
    bool seasonPosterImageChanged(int season) const;
    TvShowEpisode *episode(int season, int episode);
    QList<int> seasons();
    QList<TvShowEpisode*> episodes();
    TvShowModelItem *modelItem();
    bool hasChanged() const;
    bool infoLoaded() const;
    QString mediaCenterPath() const;
    int showId() const;
    bool downloadsInProgress() const;
    bool hasNewEpisodes() const;

    void setName(QString name);
    void setShowTitle(QString title);
    void setRating(qreal rating);
    void setFirstAired(QDate aired);
    void setGenres(QStringList genres);
    void addGenre(QString genre);
    void setCertification(QString certification);
    void setNetwork(QString network);
    void setOverview(QString overview);
    void setActors(QList<Actor> actors);
    void addActor(Actor actor);
    void setPosters(QList<Poster> posters);
    void setPoster(int index, Poster poster);
    void addPoster(Poster poster);
    void setPosterImage(QImage poster);
    void setBackdrops(QList<Poster> backdrops);
    void setBackdrop(int index, Poster backdrop);
    void addBackdrop(Poster backdrop);
    void setBackdropImage(QImage backdrop);
    void setBanners(QList<Poster> banners);
    void setBanner(int index, Poster poster);
    void addBanner(Poster banner);
    void setBannerImage(QImage banner);
    void addSeasonPoster(int season, Poster poster);
    void setSeasonPosterImage(int season, QImage poster);
    void setChanged(bool changed);
    void setModelItem(TvShowModelItem *item);
    void setMediaCenterPath(QString path);
    void setDownloadsInProgress(bool inProgress);

    void removeActor(Actor *actor);
    void removeGenre(QString *genre);

    bool loadData(MediaCenterInterface *mediaCenterInterface);
    void loadData(QString id, TvScraperInterface *tvScraperInterface, bool updateAllEpisodes);
    bool saveData(MediaCenterInterface *mediaCenterInterface);
    void loadImages(MediaCenterInterface *mediaCenterInterface);

    void scraperLoadDone();

signals:
    void sigLoaded(TvShow*);
    void sigChanged(TvShow*);

private:
    QList<TvShowEpisode*> m_episodes;
    QString m_dir;
    QString m_name;
    QString m_showTitle;
    qreal m_rating;
    QDate m_firstAired;
    QStringList m_genres;
    QString m_certification;
    QString m_network;
    QString m_overview;
    QList<Actor> m_actors;
    QList<Poster> m_posters;
    QList<Poster> m_backdrops;
    QList<Poster> m_banners;
    QMap<int, QList<Poster> > m_seasonPosters;
    QImage m_posterImage;
    QImage m_backdropImage;
    QImage m_bannerImage;
    bool m_posterImageChanged;
    bool m_backdropImageChanged;
    bool m_bannerImageChanged;
    QMap<int, QImage> m_seasonPosterImages;
    QList<int> m_seasonPosterImagesChanged;
    TvShowModelItem *m_modelItem;
    QString m_mediaCenterPath;
    int m_showId;
    bool m_downloadsInProgress;
    bool m_infoLoaded;
    bool m_hasChanged;
};

QDebug operator<<(QDebug dbg, const TvShow &show);
QDebug operator<<(QDebug dbg, const TvShow *show);

Q_DECLARE_METATYPE(TvShow*)

#endif // TVSHOW_H
