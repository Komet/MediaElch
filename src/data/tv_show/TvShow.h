#pragma once

#include "data/Actor.h"
#include "data/Locale.h"
#include "data/Poster.h"
#include "data/Rating.h"
#include "data/TmdbId.h"
#include "data/TvDbId.h"
#include "data/TvMazeId.h"
#include "data/tv_show/EpisodeNumber.h"
#include "data/tv_show/SeasonNumber.h"
#include "data/tv_show/TvShowEpisode.h"
#include "database/DatabaseId.h"
#include "globals/Globals.h"
#include "media/Path.h"
#include "scrapers/tv_show/ShowIdentifier.h"

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

    struct Exporter;
    /// \brief Write all fields to the given exporter.
    /// \see TvShow::Exporter
    void exportTo(Exporter& exporter) const;

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
    Ratings& ratings();
    const Ratings& ratings() const;
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
    /// \brief Returns all certifications for the TV show's episodes.
    QSet<Certification> episodeCertifications() const;

    const Actors& actors() const;
    Actors& actors();

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
    mediaelch::DatabaseId databaseId() const;
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
    void setDatabaseId(mediaelch::DatabaseId id);
    void setSyncNeeded(bool syncNeeded);
    void setHasTune(bool hasTune);
    void setRuntime(std::chrono::minutes runtime);
    void setShowMissingEpisodes(bool showMissing, bool updateDatabase = true);
    void setHideSpecialsInMissingEpisodes(bool hideSpecials, bool updateDatabase = true);

    void removeActor(Actor* actor);
    void removeGenre(QString genre);
    void removeTag(QString tag);

    bool loadData(MediaCenterInterface* mediaCenterInterface, bool reloadFromNfo = true, bool force = false);
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
    static QSet<ImageType> imageTypes();
    static QSet<ImageType> seasonImageTypes();

    void setDir(const mediaelch::DirectoryPath& dir);

    QString status() const;
    void setStatus(const QString& status);

    QDateTime dateAdded() const;
    void setDateAdded(const QDateTime& dateTime);

    const QMap<SeasonNumber, QString>& seasonNameMappings() const;
    void setSeasonName(SeasonNumber season, const QString& name);
    void clearSeasonName(SeasonNumber season);

public:
    /// \brief   Export interface for TvShow::exportTo
    /// \details Implement this interface and pass instances of it to TvShow::exportTo()
    ///          to export the TV show's data.  By using this Exporter, you ensure that
    ///          you will get notified of new fields (due to compilation errors).
    /// \todo    This structure does not export _everything_, for example m_nfoContent,
    ///          since that should not be part of a TV show's data, but is more of an
    ///          implementation detail.  The TvShow class needs some refactoring.
    struct Exporter
    {
        virtual void startExport() = 0;
        virtual void endExport() = 0;

        virtual void exportShowId(int showId) = 0;
        virtual void exportDatabaseId(const mediaelch::DatabaseId& databaseId) = 0;
        virtual void exportTmdbId(const TmdbId& tmdbId) = 0;
        virtual void exportTvdbId(const TvDbId& tvdbId) = 0;
        virtual void exportImdbId(const ImdbId& imdbId) = 0;
        virtual void exportTvmazeId(const TvMazeId& tvmazeId) = 0;

        virtual void exportTitle(const QString& title) = 0;
        virtual void exportShowTitle(const QString& showTitle) = 0;
        virtual void exportOriginalTitle(const QString& originalTitle) = 0;
        virtual void exportSortTitle(const QString& sortTitle) = 0;

        virtual void exportOverview(const QString& overview) = 0;
        virtual void exportRatings(const Ratings& ratings) = 0;
        virtual void exportUserRating(double userRating) = 0;
        virtual void exportImdbTop250(int imdbTop250) = 0;
        virtual void exportFirstAired(const QDate& firstAired) = 0;
        virtual void exportRuntime(const std::chrono::minutes& runtime) = 0;
        virtual void exportGenres(const QStringList& genres) = 0;
        virtual void exportTags(const QStringList& tags) = 0;
        virtual void exportCertification(const Certification& certification) = 0;
        virtual void exportNetwork(const QString& network) = 0;
        virtual void exportEpisodeGuideUrl(const QString& episodeGuideUrl) = 0;
        virtual void exportActors(const Actors& actors) = 0;
        virtual void exportPosters(const QVector<Poster>& posters) = 0;
        virtual void exportBackdrops(const QVector<Poster>& backdrops) = 0;
        virtual void exportBanners(const QVector<Poster>& banners) = 0;
        virtual void exportSeasonPosters(const QMap<SeasonNumber, QVector<Poster>>& seasonPosters) = 0;
        virtual void exportSeasonBackdrops(const QMap<SeasonNumber, QVector<Poster>>& seasonBackdrops) = 0;
        virtual void exportSeasonBanners(const QMap<SeasonNumber, QVector<Poster>>& seasonBanners) = 0;
        virtual void exportSeasonThumbs(const QMap<SeasonNumber, QVector<Poster>>& seasonThumbs) = 0;
        virtual void exportHasTune(bool hasTune) = 0;

        virtual void exportExtraFanarts(const QStringList& extraFanarts) = 0;
        virtual void exportStatus(const QString& status) = 0;
        virtual void exportDateAdded(const QDateTime& dateAdded) = 0;
        virtual void exportSeasonNameMappings(const QMap<SeasonNumber, QString>& seasonNameMappings) = 0;

        virtual void exportDir(const mediaelch::DirectoryPath& dir) = 0;
    };

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
    Ratings m_ratings;
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
    Actors m_actors;
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
    mediaelch::DatabaseId m_databaseId;
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
