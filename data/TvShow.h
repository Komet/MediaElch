#ifndef TVSHOW_H
#define TVSHOW_H

#include <QMetaType>
#include <QObject>
#include <QStringList>
#include "data/MediaCenterInterface.h"
#include "data/TvScraperInterface.h"
#include "data/TvShowEpisode.h"
#include "globals/Globals.h"

struct Actor;
struct Poster;
class MediaCenterInterface;
class TvShowEpisode;
class TvShowModelItem;
class TvScraperInterface;

/**
 * @brief The TvShow class
 */
class TvShow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString showTitle READ name WRITE setShowTitle)
    Q_PROPERTY(qreal rating READ rating WRITE setRating)
    Q_PROPERTY(QDate firstAired READ firstAired WRITE setFirstAired)
    Q_PROPERTY(QStringList genres READ genres WRITE setGenres)
    Q_PROPERTY(QString certification READ certification WRITE setCertification)
    Q_PROPERTY(QString network READ network WRITE setNetwork)
    Q_PROPERTY(QString overview READ overview WRITE setOverview)
    Q_PROPERTY(QList<Actor> actors READ actors WRITE setActors)
    Q_PROPERTY(QList<Poster> posters READ posters WRITE setPosters)
    Q_PROPERTY(QList<Poster> backdrops READ backdrops WRITE setBackdrops)
    Q_PROPERTY(QList<Poster> banners READ banners WRITE setBanners)
    Q_PROPERTY(QString tvdbId READ tvdbId WRITE setTvdbId)
    Q_PROPERTY(QString episodeGuideUrl READ episodeGuideUrl WRITE setEpisodeGuideUrl)

public:
    explicit TvShow(QString dir = QString(), QObject *parent = 0);
    void clear();
    void clear(QList<int> infos);
    void addEpisode(TvShowEpisode *episode);
    int episodeCount();

    QString name() const;
    QString showTitle() const;
    QString dir() const;
    qreal rating() const;
    QDate firstAired() const;
    QStringList genres() const;
    QStringList tags() const;
    QList<QString*> genresPointer();
    QString certification() const;
    QString network() const;
    QString overview() const;
    QString tvdbId() const;
    QString id() const;
    QString imdbId() const;
    QString episodeGuideUrl() const;
    QStringList certifications() const;
    QList<Actor> actors() const;
    QList<Actor*> actorsPointer();
    QList<Poster> posters() const;
    QList<Poster> backdrops() const;
    QList<Poster> banners() const;
    QList<Poster> seasonPosters(int season) const;
    QList<Poster> seasonBackdrops(int season) const;
    QList<Poster> seasonBanners(int season, bool returnAll = false) const;
    QList<Poster> seasonThumbs(int season, bool returnAll = false) const;
    bool posterImageChanged() const;
    bool backdropImageChanged() const;
    bool bannerImageChanged() const;
    bool logoImageChanged() const;
    bool thumbImageChanged() const;
    bool clearArtImageChanged() const;
    bool characterArtImageChanged() const;
    bool seasonPosterImageChanged(int season) const;
    bool seasonBackdropImageChanged(int season) const;
    bool seasonBannerImageChanged(int season) const;
    bool seasonThumbImageChanged(int season) const;
    TvShowEpisode *episode(int season, int episode);
    QList<int> seasons();
    QList<TvShowEpisode*> episodes();
    QList<TvShowEpisode*> episodes(int season);
    TvShowModelItem *modelItem();
    bool hasChanged() const;
    bool infoLoaded() const;
    QString mediaCenterPath() const;
    int showId() const;
    bool downloadsInProgress() const;
    bool hasNewEpisodes() const;
    bool hasNewEpisodesInSeason(QString season) const;
    QString nfoContent() const;
    int databaseId() const;
    bool syncNeeded() const;
    QList<int> infosToLoad() const;
    bool hasTune() const;

    void setName(QString name);
    void setShowTitle(QString title);
    void setRating(qreal rating);
    void setFirstAired(QDate aired);
    void setGenres(QStringList genres);
    void addGenre(QString genre);
    void addTag(QString tag);
    void setCertification(QString certification);
    void setNetwork(QString network);
    void setOverview(QString overview);
    void setTvdbId(QString id);
    void setId(QString id);
    void setImdbId(QString id);
    void setEpisodeGuideUrl(QString url);
    void setActors(QList<Actor> actors);
    void addActor(Actor actor);
    void setPosters(QList<Poster> posters);
    void setPoster(int index, Poster poster);
    void addPoster(Poster poster);
    void setBackdrops(QList<Poster> backdrops);
    void setBackdrop(int index, Poster backdrop);
    void addBackdrop(Poster backdrop);
    void setBanners(QList<Poster> banners);
    void setBanner(int index, Poster poster);
    void addBanner(Poster banner);
    void addSeasonPoster(int season, Poster poster);
    void addSeasonBackdrop(int season, Poster poster);
    void addSeasonBanner(int season, Poster poster);
    void addSeasonThumb(int season, Poster poster);
    void setChanged(bool changed);
    void setModelItem(TvShowModelItem *item);
    void setMediaCenterPath(QString path);
    void setDownloadsInProgress(bool inProgress);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
    void setSyncNeeded(bool syncNeeded);
    void setHasTune(bool hasTune);

    void removeActor(Actor *actor);
    void removeGenre(QString genre);
    void removeTag(QString tag);

    bool loadData(MediaCenterInterface *mediaCenterInterface, bool reloadFromNfo = true);
    void loadData(QString id, TvScraperInterface *tvScraperInterface, TvShowUpdateType type, QList<int> infosToLoad);
    bool saveData(MediaCenterInterface *mediaCenterInterface);
    void clearImages();

    // Images
    QByteArray posterImage();
    QByteArray backdropImage();
    QByteArray bannerImage();
    QByteArray logoImage();
    QByteArray thumbImage();
    QByteArray clearArtImage();
    QByteArray characterArtImage();
    QByteArray seasonPosterImage(int season);
    QByteArray seasonBackdropImage(int season);
    QByteArray seasonBannerImage(int season);
    QByteArray seasonThumbImage(int season);
    void setPosterImage(QByteArray poster);
    void setBackdropImage(QByteArray backdrop);
    void setBannerImage(QByteArray banner);
    void setLogoImage(QByteArray img);
    void setThumbImage(QByteArray img);
    void setClearArtImage(QByteArray img);
    void setCharacterArtImage(QByteArray img);
    void setSeasonPosterImage(int season, QByteArray poster);
    void setSeasonBackdropImage(int season, QByteArray poster);
    void setSeasonBannerImage(int season, QByteArray poster);
    void setSeasonThumbImage(int season, QByteArray poster);
    void removeImage(int type, int season = -2);
    QMap<int, QList<int> > imagesToRemove() const;

    // Extra Fanarts
    QList<ExtraFanart> extraFanarts(MediaCenterInterface *mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QList<QByteArray> extraFanartImagesToAdd();
    void addExtraFanart(QByteArray fanart);
    void removeExtraFanart(QByteArray fanart);
    void removeExtraFanart(QString file);
    void clearExtraFanartData();

    void scraperLoadDone();

    static bool lessThan(TvShow *a, TvShow *b);

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
    QStringList m_tags;
    QString m_certification;
    QString m_network;
    QString m_overview;
    QString m_tvdbId;
    QString m_id;
    QString m_imdbId;
    QString m_episodeGuideUrl;
    QList<Actor> m_actors;
    QList<Poster> m_posters;
    QList<Poster> m_backdrops;
    QList<Poster> m_banners;
    QMap<int, QList<Poster> > m_seasonPosters;
    QMap<int, QList<Poster> > m_seasonBackdrops;
    QMap<int, QList<Poster> > m_seasonBanners;
    QMap<int, QList<Poster> > m_seasonThumbs;
    QByteArray m_posterImage;
    QByteArray m_backdropImage;
    QByteArray m_bannerImage;
    QByteArray m_logoImage;
    QByteArray m_thumbImage;
    QByteArray m_clearArtImage;
    QByteArray m_characterArtImage;
    bool m_posterImageChanged;
    bool m_backdropImageChanged;
    bool m_bannerImageChanged;
    bool m_logoImageChanged;
    bool m_thumbImageChanged;
    bool m_clearArtImageChanged;
    bool m_characterArtImageChanged;
    bool m_hasTune;
    QMap<int, QByteArray> m_seasonPosterImages;
    QList<int> m_seasonPosterImagesChanged;
    QMap<int, QByteArray> m_seasonBackdropImages;
    QList<int> m_seasonBackdropImagesChanged;
    QMap<int, QByteArray> m_seasonBannerImages;
    QList<int> m_seasonBannerImagesChanged;
    QMap<int, QByteArray> m_seasonThumbImages;
    QList<int> m_seasonThumbImagesChanged;
    TvShowModelItem *m_modelItem;
    QString m_mediaCenterPath;
    int m_showId;
    bool m_downloadsInProgress;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    bool m_hasChanged;
    QString m_nfoContent;
    int m_databaseId;
    bool m_syncNeeded;
    QList<int> m_infosToLoad;
    QList<QByteArray> m_extraFanartImagesToAdd;
    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    QMap<int, QList<int> > m_imagesToRemove;
};

QDebug operator<<(QDebug dbg, const TvShow &show);
QDebug operator<<(QDebug dbg, const TvShow *show);

Q_DECLARE_METATYPE(TvShow*)

#endif // TVSHOW_H
