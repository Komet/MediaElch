#ifndef TVSHOW_H
#define TVSHOW_H

#include <QMetaType>
#include <QObject>
#include <QStringList>

#include "data/MediaCenterInterface.h"
#include "data/Rating.h"
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

public:
    explicit TvShow(QString dir = QString(), QObject *parent = nullptr);
    void clear();
    void clear(QList<TvShowScraperInfos> infos);
    void addEpisode(TvShowEpisode *episode);
    virtual int episodeCount() const;

    virtual QString name() const;
    virtual QString showTitle() const;
    virtual QString dir() const;
    virtual double rating() const;
    virtual int votes() const;
    virtual int top250() const;
    virtual QDate firstAired() const;
    virtual QStringList genres() const;
    virtual QStringList tags() const;
    virtual QList<QString *> genresPointer();
    virtual QString certification() const;
    virtual QString network() const;
    virtual QString overview() const;
    virtual QString tvdbId() const;
    virtual QString id() const;
    virtual QString imdbId() const;
    virtual QString episodeGuideUrl() const;
    virtual QStringList certifications() const;
    virtual QList<Actor> actors() const;
    virtual QList<Actor *> actorsPointer();
    virtual QList<Poster> posters() const;
    virtual QList<Poster> backdrops() const;
    virtual QList<Poster> banners() const;
    virtual QList<Poster> seasonPosters(int season) const;
    virtual QList<Poster> seasonBackdrops(int season) const;
    virtual QList<Poster> seasonBanners(int season, bool returnAll = false) const;
    virtual QList<Poster> seasonThumbs(int season, bool returnAll = false) const;
    virtual TvShowEpisode *episode(int season, int episode);
    virtual QList<int> seasons(bool includeDummies = true) const;
    virtual QList<TvShowEpisode *> episodes() const;
    virtual QList<TvShowEpisode *> episodes(int season) const;
    virtual TvShowModelItem *modelItem();
    virtual bool hasChanged() const;
    virtual bool infoLoaded() const;
    virtual QString mediaCenterPath() const;
    virtual int showId() const;
    virtual bool downloadsInProgress() const;
    virtual bool hasNewEpisodes() const;
    virtual bool hasNewEpisodesInSeason(int season) const;
    virtual QString nfoContent() const;
    virtual int databaseId() const;
    virtual bool syncNeeded() const;
    virtual QList<TvShowScraperInfos> infosToLoad() const;
    virtual bool hasTune() const;
    virtual int runtime() const;
    virtual QString sortTitle() const;
    virtual bool isDummySeason(int season) const;
    virtual bool hasDummyEpisodes() const;
    virtual bool hasDummyEpisodes(int season) const;
    virtual bool showMissingEpisodes() const;
    virtual bool hideSpecialsInMissingEpisodes() const;

    void setName(QString name);
    void setShowTitle(QString title);
    void setRating(double rating);
    void setVotes(int votes);
    void setTop250(int top250);
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
    void setRuntime(int runtime);
    void setSortTitle(QString sortTitle);
    void setShowMissingEpisodes(bool showMissing, bool updateDatabase = true);
    void setHideSpecialsInMissingEpisodes(bool hideSpecials, bool updateDatabase = true);

    void removeActor(Actor *actor);
    void removeGenre(QString genre);
    void removeTag(QString tag);

    bool loadData(MediaCenterInterface *mediaCenterInterface, bool reloadFromNfo = true);
    void loadData(QString id,
        TvScraperInterface *tvScraperInterface,
        TvShowUpdateType type,
        QList<TvShowScraperInfos> infosToLoad);
    bool saveData(MediaCenterInterface *mediaCenterInterface);
    void clearImages();
    void fillMissingEpisodes();
    void clearMissingEpisodes();

    // Images
    void removeImage(ImageType type, int season = -2);
    QMap<ImageType, QList<int>> imagesToRemove() const;
    QByteArray image(ImageType imageType);
    QByteArray seasonImage(int season, ImageType imageType);
    void setImage(ImageType imageType, QByteArray image);
    void setSeasonImage(int season, ImageType imageType, QByteArray image);
    bool imageHasChanged(ImageType imageType) const;
    bool seasonImageHasChanged(int season, ImageType imageType) const;
    bool hasImage(ImageType type);

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
    static QList<ImageType> imageTypes();
    static QList<ImageType> seasonImageTypes();

    void setDir(const QString &dir);

    QString status() const;
    void setStatus(const QString &status);

signals:
    void sigLoaded(TvShow *);
    void sigChanged(TvShow *);

private:
    QList<TvShowEpisode *> m_episodes;
    QString m_dir;
    QString m_name;
    QString m_showTitle;
    QString m_sortTitle;
    Rating m_rating;
    QDate m_firstAired;
    int m_runtime;
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
    QMap<int, QList<Poster>> m_seasonPosters;
    QMap<int, QList<Poster>> m_seasonBackdrops;
    QMap<int, QList<Poster>> m_seasonBanners;
    QMap<int, QList<Poster>> m_seasonThumbs;
    bool m_hasTune;
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
    QList<TvShowScraperInfos> m_infosToLoad;
    QList<QByteArray> m_extraFanartImagesToAdd;
    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    QMap<ImageType, QList<int>> m_imagesToRemove;
    QMap<ImageType, bool> m_hasImage;
    bool m_showMissingEpisodes;
    bool m_hideSpecialsInMissingEpisodes;
    QString m_status;

    QMap<ImageType, QByteArray> m_images;
    QMap<int, QMap<ImageType, QByteArray>> m_seasonImages;
    QMap<ImageType, bool> m_hasImageChanged;
    QMap<int, QMap<ImageType, bool>> m_hasSeasonImageChanged;

    void clearSeasonImageType(ImageType imageType);
};

QDebug operator<<(QDebug dbg, const TvShow &show);
QDebug operator<<(QDebug dbg, const TvShow *show);

Q_DECLARE_METATYPE(TvShow *)

#endif // TVSHOW_H
