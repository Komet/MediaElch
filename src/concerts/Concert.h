#pragma once

#include "concerts/ConcertController.h"
#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Rating.h"
#include "data/TmdbId.h"
#include "globals/Globals.h"

#include <QByteArray>
#include <QDate>
#include <QDebug>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QStringList>
#include <QUrl>
#include <chrono>

class MediaCenterInterface;
class StreamDetails;

/**
 * @brief The Concert class
 * This class represents a single concert
 */
class Concert final : public QObject
{
    Q_OBJECT

public:
    explicit Concert(QStringList files, QObject* parent = nullptr);
    ~Concert() = default;

    ConcertController* controller() const;

    void clear();
    void clear(QVector<ConcertScraperInfos> infos);

    QString name() const;
    QString artist() const;
    QString album() const;
    QString overview() const;
    Rating rating() const;
    QDate released() const;
    QString tagline() const;
    std::chrono::minutes runtime() const;
    Certification certification() const;
    QStringList genres() const;
    QStringList tags() const;
    QVector<QString*> genresPointer();
    QUrl trailer() const;
    QStringList files() const;
    QString folderName() const;
    int playcount() const;
    QDateTime lastPlayed() const;
    QVector<Poster> posters() const;
    QVector<Poster> backdrops() const;
    bool watched() const;
    int concertId() const;
    bool downloadsInProgress() const;
    int downloadsSize() const;
    bool inSeparateFolder() const;
    int mediaCenterId() const;
    TmdbId tmdbId() const;
    ImdbId imdbId() const;
    StreamDetails* streamDetails() const;
    bool streamDetailsLoaded() const;
    QString nfoContent() const;
    int databaseId() const;
    bool syncNeeded() const;

    bool hasChanged() const;

    void setFiles(QStringList files);
    void setName(QString name);
    void setArtist(QString artist);
    void setAlbum(QString album);
    void setOverview(QString overview);
    void setRating(Rating rating);
    void setReleased(QDate released);
    void setTagline(QString tagline);
    void setRuntime(std::chrono::minutes runtime);
    void setCertification(Certification cert);
    void setTrailer(QUrl trailer);
    void addGenre(QString genre);
    void addTag(QString tag);
    void setPlayCount(int playcount);
    void setLastPlayed(QDateTime lastPlayed);
    void setPosters(QVector<Poster> posters);
    void setPoster(int index, Poster poster);
    void addPoster(Poster poster);
    void setBackdrops(QVector<Poster> backdrops);
    void setBackdrop(int index, Poster backdrop);
    void addBackdrop(Poster backdrop);
    void setWatched(bool watched);
    void setChanged(bool changed);
    void setDownloadsInProgress(bool inProgress);
    void setDownloadsSize(int downloadsSize);
    void setInSeparateFolder(bool inSepFolder);
    void setMediaCenterId(int mediaCenterId);
    void setTmdbId(TmdbId id);
    void setImdbId(ImdbId id);
    void setStreamDetailsLoaded(bool loaded);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
    void setSyncNeeded(bool syncNeeded);

    void removeGenre(QString genre);
    void removeTag(QString tag);

    // Extra Fanarts
    QVector<ExtraFanart> extraFanarts(MediaCenterInterface* mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QVector<QByteArray> extraFanartImagesToAdd();
    void addExtraFanart(QByteArray fanart);
    void removeExtraFanart(QByteArray fanart);
    void removeExtraFanart(QString file);
    void clearExtraFanartData();

    void clearImages();
    void removeImage(ImageType type);
    QVector<ImageType> imagesToRemove() const;

    QByteArray image(ImageType imageType);
    bool imageHasChanged(ImageType imageType);
    void setImage(ImageType imageType, QByteArray image);
    void setHasImage(ImageType imageType, bool has);
    bool hasImage(ImageType imageType);
    bool hasExtraFanarts() const;
    void setHasExtraFanarts(bool has);

    void scraperLoadDone();
    QVector<ConcertScraperInfos> infosToLoad();
    void setLoadsLeft(QVector<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);

    void setDiscType(DiscType type);
    DiscType discType() const;

    static bool lessThan(Concert* a, Concert* b);
    static QVector<ImageType> imageTypes();

signals:
    void sigChanged(Concert*);

private:
    ConcertController* m_controller;
    QStringList m_files;
    QString m_folderName;
    QString m_name;
    QString m_artist;
    QString m_album;
    QString m_overview;
    Rating m_rating;
    QDate m_released;
    QString m_tagline;
    std::chrono::minutes m_runtime;
    Certification m_certification;
    QStringList m_genres;
    QStringList m_tags;
    QUrl m_trailer;
    int m_playcount;
    QDateTime m_lastPlayed;
    QVector<Poster> m_posters;
    QVector<Poster> m_backdrops;
    int m_concertId;
    int m_downloadsSize;
    bool m_watched;
    bool m_hasChanged;
    bool m_downloadsInProgress;
    bool m_inSeparateFolder;
    int m_mediaCenterId;
    TmdbId m_tmdbId;
    ImdbId m_imdbId;
    QVector<ConcertScraperInfos> m_infosToLoad;
    bool m_streamDetailsLoaded;
    StreamDetails* m_streamDetails;
    QString m_nfoContent;
    int m_databaseId;
    bool m_syncNeeded;
    QVector<ScraperData> m_loadsLeft;
    QMutex m_loadMutex;
    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    bool m_hasExtraFanarts;

    QMap<ImageType, QByteArray> m_images;
    QMap<ImageType, bool> m_hasImageChanged;
    QVector<QByteArray> m_extraFanartImagesToAdd;
    QVector<ImageType> m_imagesToRemove;
    QMap<ImageType, bool> m_hasImage;
};
