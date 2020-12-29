#pragma once

#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Locale.h"
#include "data/Rating.h"
#include "data/StreamDetails.h"
#include "data/TmdbId.h"
#include "file/Path.h"
#include "globals/Actor.h"
#include "globals/Globals.h"
#include "globals/ScraperInfos.h"
#include "scrapers/tv_show/ShowIdentifier.h"
#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/SeasonOrder.h"
#include "tv_shows/TvDbId.h"
#include "tv_shows/TvMazeId.h"

#include <QMetaType>
#include <QObject>
#include <QStringList>
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

    void setFiles(const mediaelch::FileList& files);
    TvShow* tvShow() const;
    const mediaelch::FileList& files() const;

    /// \brief Title of the show this episode belongs to.
    QString showTitle() const;
    /// \brief This episode's title.
    QString title() const;
    /// \brief Episode title with prepended "SXXEXX "
    QString completeEpisodeName() const;

    QVector<Rating>& ratings();
    const QVector<Rating>& ratings() const;
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
    QString network() const;
    QString seasonString() const;
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
    int databaseId() const;
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
    void setNetwork(QString network);
    void setThumbnail(QUrl url);
    void setThumbnailImage(QByteArray thumbnail);
    void setEpBookmark(QTime epBookmark);
    void setInfosLoaded(bool loaded);
    void setChanged(bool changed);
    void setModelItem(EpisodeModelItem* item);
    void setStreamDetailsLoaded(bool loaded);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
    void setSyncNeeded(bool syncNeeded);
    void setIsDummy(bool dummy);
    void setWantThumbnailDownload(bool wantThumbnail);

    void removeWriter(QString* writer);
    void removeDirector(QString* director);
    void removeTag(QString tag);

    QVector<const Actor*> actors() const;
    QVector<Actor*> actors();
    void addActor(Actor actor);
    void removeActor(Actor* actor);

    bool loadData(MediaCenterInterface* mediaCenterInterface, bool reloadFromNfo, bool forceReload);
    bool saveData(MediaCenterInterface* mediaCenterInterface);
    void scrapeData(mediaelch::scraper::TvScraper* scraper,
        mediaelch::Locale locale,
        const mediaelch::scraper::ShowIdentifier& showIdentifier,
        SeasonOrder order,
        const QSet<EpisodeScraperInfo>& infosToLoad);
    void loadStreamDetailsFromFile();
    void clearImages();
    QSet<EpisodeScraperInfo> infosToLoad();

    QVector<ImageType> imagesToRemove() const;
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
    QVector<Rating> m_ratings;
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
    QString m_network;
    QUrl m_thumbnail;
    QByteArray m_thumbnailImage;
    EpisodeModelItem* m_modelItem = nullptr;
    bool m_thumbnailImageChanged = false;
    bool m_infoLoaded = false;
    bool m_infoFromNfoLoaded = false;
    bool m_hasChanged = false;
    int m_episodeId = -1;
    bool m_streamDetailsLoaded = false;
    StreamDetails* m_streamDetails = nullptr;
    QString m_nfoContent;
    int m_databaseId = -1;
    bool m_syncNeeded = false;
    QSet<EpisodeScraperInfo> m_infosToLoad;
    QVector<ImageType> m_imagesToRemove;
    bool m_isDummy = false;
    std::vector<std::unique_ptr<Actor>> m_actors;
    bool m_wantThumbnailDownload = false;
};

QDebug operator<<(QDebug dbg, const TvShowEpisode& episode);
QDebug operator<<(QDebug dbg, const TvShowEpisode* episode);

Q_DECLARE_METATYPE(TvShowEpisode*)
