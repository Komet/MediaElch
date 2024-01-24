#pragma once

#include "data/Actor.h"
#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Locale.h"
#include "data/Rating.h"
#include "data/TmdbId.h"
#include "data/TvDbId.h"
#include "data/TvMazeId.h"
#include "data/tv_show/EpisodeNumber.h"
#include "data/tv_show/SeasonNumber.h"
#include "data/tv_show/SeasonOrder.h"
#include "database/DatabaseId.h"
#include "globals/Globals.h"
#include "media/Path.h"
#include "media/StreamDetails.h"
#include "scrapers/ScraperInfos.h"
#include "scrapers/tv_show/ShowIdentifier.h"

#include <QMetaType>
#include <QObject>
#include <QStringList>
#include <QUrl>
#include <memory>
#include <vector>

class MediaCenterInterface;
class StreamDetails;
class TvShow;
class EpisodeModelItem;

namespace mediaelch {
namespace scraper {
class TvScraper;
}
} // namespace mediaelch

class TvShowEpisode final : public QObject
{
    Q_OBJECT

public:
    TvShowEpisode(const mediaelch::FileList& files, TvShow* parentShow);
    explicit TvShowEpisode(const mediaelch::FileList& files = {}, QObject* parent = nullptr);
    void clear();
    void clear(const QSet<EpisodeScraperInfo>& infos);

    struct Exporter;
    /// \brief Write all fields to the given exporter.
    /// \see TvShowEpisode::Exporter
    void exportTo(Exporter& exporter) const;

    void setFiles(const mediaelch::FileList& files);
    TvShow* tvShow() const;
    const mediaelch::FileList& files() const;

    /// \brief Title of the show this episode belongs to.
    QString showTitle() const;
    /// \brief This episode's title.
    QString title() const;
    /// \brief Episode title with prepended "SXXEXX "
    QString completeEpisodeName() const;

    Ratings& ratings();
    const Ratings& ratings() const;
    double userRating() const;
    int top250() const;
    SeasonNumber seasonNumber() const;
    EpisodeNumber episodeNumber() const;
    SeasonNumber displaySeason() const;
    EpisodeNumber displayEpisode() const;
    QString overview() const;
    QStringList writers() const;
    QStringList directors() const;
    int playCount() const;
    QDateTime lastPlayed() const;
    QDate firstAired() const;
    QStringList tags() const;
    QTime epBookmark() const;
    Certification certification() const;
    QStringList networks() const;
    QString seasonString() const;
    QString seasonName() const;
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
    const StreamDetails* streamDetails() const;
    bool streamDetailsLoaded() const;
    QString nfoContent() const;
    mediaelch::DatabaseId databaseId() const;
    bool syncNeeded() const;
    bool isDummy() const;
    bool wantThumbnailDownload() const;

    void setShow(TvShow* show);
    void setTitle(QString title);
    void setShowTitle(QString showTitle);
    void setUserRating(double rating);
    void setTop250(int top250);
    void setSeason(SeasonNumber seasonNumber);
    void setEpisode(EpisodeNumber episodeNumber);
    void setDisplaySeason(SeasonNumber seasonNumber);
    void setDisplayEpisode(EpisodeNumber episodeNumber);
    void setOverview(QString overview);
    void setWriters(QStringList writers);
    void addWriter(QString writer);
    void setDirectors(QStringList directors);
    void addDirector(QString director);
    void setPlayCount(int playCount);
    void setLastPlayed(QDateTime lastPlayed);
    void setFirstAired(QDate firstAired);
    void addTag(QString tag);
    void setCertification(Certification certification);
    void setNetworks(QStringList network);
    void addNetwork(QString network);
    void setThumbnail(QUrl url);
    void setThumbnailImage(QByteArray thumbnail);
    void setEpBookmark(QTime epBookmark);
    void setInfosLoaded(bool loaded);
    void setChanged(bool changed);
    void setModelItem(EpisodeModelItem* item);
    void setNfoContent(QString content);
    void setDatabaseId(mediaelch::DatabaseId id);
    void setSyncNeeded(bool syncNeeded);
    void setIsDummy(bool dummy);
    void setWantThumbnailDownload(bool wantThumbnail);

    void removeWriter(QString* writer);
    void removeDirector(QString* director);
    void removeTag(QString tag);

    const Actors& actors() const;
    Actors& actors();
    void addActor(Actor actor);
    void removeActor(Actor* actor);

    bool loadData(MediaCenterInterface* mediaCenterInterface, bool reloadFromNfo, bool forceReload);
    bool saveData(MediaCenterInterface* mediaCenterInterface);
    void scrapeData(mediaelch::scraper::TvScraper* scraper,
        mediaelch::Locale locale,
        const mediaelch::scraper::ShowIdentifier& showIdentifier,
        SeasonOrder order,
        const QSet<EpisodeScraperInfo>& infosToLoad);

    /// \brief Tries to load streamdetails from the file
    ELCH_NODISCARD bool loadStreamDetailsFromFile();

    void clearImages();
    QSet<EpisodeScraperInfo> infosToLoad();

    QSet<ImageType> imagesToRemove() const;
    void removeImage(ImageType type);

    static bool lessThan(TvShowEpisode* a, TvShowEpisode* b);

    TmdbId tmdbId() const;
    void setTmdbId(const TmdbId& tmdbId);
    ImdbId imdbId() const;
    void setImdbId(const ImdbId& imdbId);
    TvDbId tvdbId() const;
    void setTvdbId(const TvDbId& tvdbId);
    TvMazeId tvmazeId() const;
    void setTvMazeId(const TvMazeId& tvmazeId);

public:
    /// \brief   Export interface for TvShowEpisode::exportTo().
    /// \details Implement this interface and pass instances of it to TvShowEpisode::exportTo()
    ///          to export the episode's data.  By using this Exporter, you ensure that
    ///          you will get notified of new fields (due to compilation errors).
    /// \todo    This structure does not export _everything_, for example m_nfoContent,
    ///          since that should not be part of an episode's data, but is more of an
    ///          implementation detail.  The TvShowEpisode class needs some refactoring.
    struct Exporter
    {
        virtual void startExport() = 0;
        virtual void endExport() = 0;

        virtual void exportEpisodeId(int episodeId) = 0;
        virtual void exportDatabaseId(const mediaelch::DatabaseId& databaseId) = 0;
        virtual void exportTmdbId(const TmdbId& tmdbId) = 0;
        virtual void exportImdbId(const ImdbId& imdbId) = 0;
        virtual void exportTvdbId(const TvDbId& tvdbId) = 0;
        virtual void exportTvMazeId(const TvMazeId& tvmazeId) = 0;

        virtual void exportTitle(const QString& title) = 0;
        virtual void exportShowTitle(const QString& showTitle) = 0;

        virtual void exportRatings(const Ratings& ratings) = 0;
        virtual void exportUserRating(double userRating) = 0;
        virtual void exportImdbTop250(int imdbTop250) = 0;

        virtual void exportSeason(SeasonNumber season) = 0;
        virtual void exportEpisode(EpisodeNumber episode) = 0;
        virtual void exportDisplaySeason(SeasonNumber displaySeason) = 0;
        virtual void exportDisplayEpisode(EpisodeNumber displayEpisode) = 0;

        virtual void exportOverview(const QString& overview) = 0;
        virtual void exportWriters(const QStringList& writers) = 0;
        virtual void exportDirectors(const QStringList& directors) = 0;
        virtual void exportPlayCount(int playCount) = 0;
        virtual void exportLastPlayed(const QDateTime& lastPlayed) = 0;
        virtual void exportFirstAired(const QDate& firstAired) = 0;
        virtual void exportTags(const QStringList& tags) = 0;
        virtual void exportEpBookmark(const QTime& epBookmark) = 0;
        virtual void exportCertification(const Certification& certification) = 0;
        virtual void exportNetworks(const QStringList& networks) = 0;
        virtual void exportThumbnail(const QUrl& thumbnail) = 0;
        virtual void exportActors(const Actors& actors) = 0;
        virtual void exportStreamDetails(const StreamDetails* streamDetails) = 0;
        virtual void exportFiles(const mediaelch::FileList& files) = 0;
    };

signals:
    void sigLoaded(TvShowEpisode*);
    void sigChanged(TvShowEpisode*);

private:
    void initCounter();

private:
    mediaelch::FileList m_files;
    TvShow* m_show = nullptr;
    QString m_title;
    QString m_showTitle;
    Ratings m_ratings;
    double m_userRating = 0.0;
    int m_imdbTop250 = 0;
    TmdbId m_tmdbId;
    ImdbId m_imdbId;
    TvDbId m_tvdbId;
    TvMazeId m_tvmazeId;
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
    QStringList m_tags;
    QTime m_epBookmark;
    Certification m_certification;
    QStringList m_networks;
    QUrl m_thumbnail;
    QByteArray m_thumbnailImage;
    EpisodeModelItem* m_modelItem = nullptr;
    bool m_thumbnailImageChanged = false;
    bool m_infoLoaded = false;
    bool m_infoFromNfoLoaded = false;
    bool m_hasChanged = false;
    int m_episodeId = -1;
    StreamDetails* m_streamDetails = nullptr;
    QString m_nfoContent;
    mediaelch::DatabaseId m_databaseId;
    bool m_syncNeeded = false;
    QSet<EpisodeScraperInfo> m_infosToLoad;
    QSet<ImageType> m_imagesToRemove;
    bool m_isDummy = false;
    Actors m_actors;
    bool m_wantThumbnailDownload = false;
};

QDebug operator<<(QDebug dbg, const TvShowEpisode& episode);
QDebug operator<<(QDebug dbg, const TvShowEpisode* episode);

Q_DECLARE_METATYPE(TvShowEpisode*)
