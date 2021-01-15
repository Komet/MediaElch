#pragma once

#include "data/Locale.h"
#include "data/Rating.h"
#include "data/TmdbId.h"
#include "file/Path.h"
#include "globals/Actor.h"
#include "globals/Globals.h"
#include "globals/Poster.h"
#include "scrapers/tv_show/ShowIdentifier.h"
#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/TvDbId.h"
#include "tv_shows/TvMazeId.h"
#include "tv_shows/TvShowEpisode.h"

#include <QMetaType>
#include <QObject>
#include <QStringList>
#include <QVector>
#include <chrono>
#include <memory>

class MediaCenterInterface;
class TvShowModelItem;

namespace mediaelch {
namespace scraper {
class TvScraper;
}
} // namespace mediaelch

class TvShow final : public QObject
{
    Q_OBJECT

public:
    explicit TvShow(mediaelch::DirectoryPath dir = {}, QObject* parent = nullptr);
    void clear();
    void clear(QSet<ShowScraperInfo> infos);
    void clearEpisodes(QSet<EpisodeScraperInfo> infos, bool onlyNew);
    void addEpisode(TvShowEpisode* episode);
    int episodeCount() const;

    /// \brief Main title of the show.
    QString title() const;
    /// \brief Alternate title of the show.
    /// \details Some Kodi skins may display this title instead of title().
    ///          Unused by MediaElch except for reading/writing the XML tag.
    QString showTitle() const;
    /// \brief Original title of the show, i.e. in it's native language.
    QString originalTitle() const;
    /// \brief Title used to sort TV shows. Useful when using special characters, etc.
    QString sortTitle() const;

    mediaelch::DirectoryPath dir() const;
    QVector<Rating>& ratings();
    const QVector<Rating>& ratings() const;
    double userRating() const;
    int top250() const;
    QDate firstAired() const;
    QStringList genres() const;
    QStringList tags() const;
    QVector<QString*> genresPointer();
    Certification certification() const;
    QString network() const;
    QString overview() const;
    TmdbId tmdbId() const;
    TvDbId tvdbId() const;
    ImdbId imdbId() const;
    TvMazeId tvmazeId() const;
    QString episodeGuideUrl() const;
    QVector<Certification> certifications() const;
    QVector<const Actor*> actors() const;
    QVector<Actor*> actors();
    QVector<Poster> posters() const;
    QVector<Poster> backdrops() const;
    QVector<Poster> banners() const;
    QVector<Poster> seasonPosters(SeasonNumber season, bool returnAll = false) const;
    QVector<Poster> seasonBackdrops(SeasonNumber season) const;
    QVector<Poster> seasonBanners(SeasonNumber season, bool returnAll = false) const;
    QVector<Poster> seasonThumbs(SeasonNumber season, bool returnAll = false) const;

    const QMap<SeasonNumber, QVector<Poster>>& allSeasonPosters() const;
    const QMap<SeasonNumber, QVector<Poster>>& allSeasonBackdrops() const;
    const QMap<SeasonNumber, QVector<Poster>>& allSeasonBanners() const;
    const QMap<SeasonNumber, QVector<Poster>>& allSeasonThumbs() const;

    TvShowEpisode* episode(SeasonNumber season, EpisodeNumber episode);
    QVector<SeasonNumber> seasons(bool includeDummies = true) const;
    const QVector<TvShowEpisode*>& episodes() const;
    QVector<TvShowEpisode*> episodes(SeasonNumber season) const;
    TvShowModelItem* modelItem();
    bool hasChanged() const;
    bool infoLoaded() const;
    mediaelch::DirectoryPath mediaCenterPath() const;
    int showId() const;
    bool downloadsInProgress() const;
    bool hasNewEpisodes() const;
    bool hasNewEpisodesInSeason(SeasonNumber season) const;
    QString nfoContent() const;
    int databaseId() const;
    bool syncNeeded() const;
    QSet<ShowScraperInfo> infosToLoad() const;
    QSet<EpisodeScraperInfo> episodeInfosToLoad() const;
    bool hasTune() const;
    std::chrono::minutes runtime() const;

    bool isDummySeason(SeasonNumber season) const;
    bool hasDummyEpisodes() const;
    bool hasDummyEpisodes(SeasonNumber season) const;
    bool showMissingEpisodes() const;
    bool hideSpecialsInMissingEpisodes() const;

    void setTitle(const QString& title);
    void setOriginalTitle(const QString& title);
    void setShowTitle(const QString& title);
    void setSortTitle(const QString& sortTitle);
    void setUserRating(double rating);
    void setTop250(int top250);
    void setFirstAired(QDate aired);
    void setGenres(QStringList genres);
    void addGenre(QString genre);
    void addTag(QString tag);
    void setCertification(Certification certification);
    void setNetwork(QString network);
    void setOverview(QString overview);
    void setTmdbId(TmdbId id);
    void setTvdbId(TvDbId id);
    void setImdbId(ImdbId id);
    void setTvMazeId(TvMazeId id);
    void setEpisodeGuideUrl(QString url);
    void addActor(Actor actor);
    void setPosters(QVector<Poster> posters);
    void setPoster(int index, Poster poster);
    void addPoster(Poster poster);
    void setBackdrops(QVector<Poster> backdrops);
    void setBackdrop(int index, Poster backdrop);
    void addBackdrop(Poster backdrop);
    void setBanners(QVector<Poster> banners);
    void setBanner(int index, Poster poster);
    void addBanner(Poster banner);
    void addSeasonPoster(SeasonNumber season, Poster poster);
    void addSeasonBackdrop(SeasonNumber season, Poster poster);
    void addSeasonBanner(SeasonNumber season, Poster poster);
    void addSeasonThumb(SeasonNumber season, Poster poster);
    void setChanged(bool changed);
    void setModelItem(TvShowModelItem* item);
    void setMediaCenterPath(mediaelch::DirectoryPath path);
    void setDownloadsInProgress(bool inProgress);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
    void setSyncNeeded(bool syncNeeded);
    void setHasTune(bool hasTune);
    void setRuntime(std::chrono::minutes runtime);
    void setShowMissingEpisodes(bool showMissing, bool updateDatabase = true);
    void setHideSpecialsInMissingEpisodes(bool hideSpecials, bool updateDatabase = true);

    void removeActor(Actor* actor);
    void removeGenre(QString genre);
    void removeTag(QString tag);

    bool loadData(MediaCenterInterface* mediaCenterInterface, bool reloadFromNfo = true);
    bool saveData(MediaCenterInterface* mediaCenterInterface);
    void scrapeData(mediaelch::scraper::TvScraper* scraper,
        const mediaelch::scraper::ShowIdentifier& id,
        const mediaelch::Locale& locale,
        SeasonOrder order,
        TvShowUpdateType updateType,
        const QSet<ShowScraperInfo>& showDetails,
        const QSet<EpisodeScraperInfo>& episodedetails);
    void clearImages();
    void fillMissingEpisodes();
    void clearMissingEpisodes();

    // Images
    void removeImage(ImageType type, SeasonNumber season = SeasonNumber::NoSeason);
    QMap<ImageType, QVector<SeasonNumber>> imagesToRemove() const;
    QByteArray image(ImageType imageType);
    QByteArray seasonImage(SeasonNumber season, ImageType imageType);
    void setImage(ImageType imageType, QByteArray image);
    void setSeasonImage(SeasonNumber season, ImageType imageType, QByteArray image);
    bool imageHasChanged(ImageType imageType) const;
    bool seasonImageHasChanged(SeasonNumber season, ImageType imageType) const;
    bool hasImage(ImageType type);

    // Extra Fanarts
    QVector<ExtraFanart> extraFanarts(MediaCenterInterface* mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QVector<QByteArray> extraFanartImagesToAdd();
    void addExtraFanart(QByteArray fanart);
    void removeExtraFanart(QByteArray fanart);
    void removeExtraFanart(QString file);
    void clearExtraFanartData();

    static bool lessThan(TvShow* a, TvShow* b);
    static QVector<ImageType> imageTypes();
    static QVector<ImageType> seasonImageTypes();

    void setDir(const mediaelch::DirectoryPath& dir);

    QString status() const;
    void setStatus(const QString& status);

    QDateTime dateAdded() const;
    void setDateAdded(const QDateTime& dateTime);

    const QMap<SeasonNumber, QString>& seasonNameMappings() const;
    void setSeasonName(SeasonNumber season, const QString& name);

signals:
    /// \todo Remove in future versions. TV show should not know about its scrapers.
    void sigLoaded(TvShow* show, QSet<ShowScraperInfo> details, mediaelch::Locale locale);
    void sigChanged(TvShow*);

private:
    QVector<TvShowEpisode*> m_episodes;
    mediaelch::DirectoryPath m_dir;
    QString m_title;
    QString m_showTitle;
    QString m_originalTitle;
    QString m_sortTitle;
    QVector<Rating> m_ratings;
    double m_userRating = 0.0;
    int m_imdbTop250 = 0;
    QDate m_firstAired;
    std::chrono::minutes m_runtime;
    QStringList m_genres;
    QStringList m_tags;
    Certification m_certification;
    QString m_network;
    QString m_overview;
    TmdbId m_tmdbId;
    TvDbId m_tvdbId;
    ImdbId m_imdbId;
    TvMazeId m_tvmazeId;
    QString m_episodeGuideUrl;
    std::vector<std::unique_ptr<Actor>> m_actors;
    QVector<Poster> m_posters;
    QVector<Poster> m_backdrops;
    QVector<Poster> m_banners;
    QMap<SeasonNumber, QVector<Poster>> m_seasonPosters;
    QMap<SeasonNumber, QVector<Poster>> m_seasonBackdrops;
    QMap<SeasonNumber, QVector<Poster>> m_seasonBanners;
    QMap<SeasonNumber, QVector<Poster>> m_seasonThumbs;
    bool m_hasTune = false;
    TvShowModelItem* m_modelItem = nullptr;
    mediaelch::DirectoryPath m_mediaCenterPath;
    int m_showId = -1;
    bool m_downloadsInProgress = false;
    bool m_infoLoaded = false;
    bool m_infoFromNfoLoaded = false;
    bool m_hasChanged = false;
    QString m_nfoContent;
    int m_databaseId = -1;
    bool m_syncNeeded = false;
    /// \todo Remove in future versions.
    QSet<ShowScraperInfo> m_infosToLoad;
    QSet<EpisodeScraperInfo> m_episodeInfosToLoad;
    QVector<QByteArray> m_extraFanartImagesToAdd;
    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    QMap<ImageType, QVector<SeasonNumber>> m_imagesToRemove;
    QMap<ImageType, bool> m_hasImage;
    bool m_showMissingEpisodes = false;
    bool m_hideSpecialsInMissingEpisodes = false;
    QString m_status;
    QDateTime m_dateAdded;
    QMap<SeasonNumber, QString> m_seasonNameMappings;

    QMap<ImageType, QByteArray> m_images;
    QMap<SeasonNumber, QMap<ImageType, QByteArray>> m_seasonImages;
    QMap<ImageType, bool> m_hasImageChanged;
    QMap<SeasonNumber, QMap<ImageType, bool>> m_hasSeasonImageChanged;

    void clearSeasonImageType(ImageType imageType);
};

QDebug operator<<(QDebug dbg, const TvShow& show);
QDebug operator<<(QDebug dbg, const TvShow* show);

Q_DECLARE_METATYPE(TvShow*)
