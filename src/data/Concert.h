#ifndef CONCERT_H
#define CONCERT_H

#include "concerts/ConcertController.h"
#include "data/ImdbId.h"
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
class Concert : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString artist READ artist WRITE setArtist)
    Q_PROPERTY(QString album READ album WRITE setAlbum)
    Q_PROPERTY(qreal rating READ rating WRITE setRating)
    Q_PROPERTY(QDate released READ released WRITE setReleased)
    Q_PROPERTY(QString overview READ overview WRITE setOverview)
    Q_PROPERTY(QString tagline READ tagline WRITE setTagline)
    Q_PROPERTY(std::chrono::minutes runtime READ runtime WRITE setRuntime)
    Q_PROPERTY(QString certification READ certification WRITE setCertification)
    Q_PROPERTY(int playcount READ playcount WRITE setPlayCount)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed WRITE setLastPlayed)
    Q_PROPERTY(QUrl trailer READ trailer WRITE setTrailer)
    Q_PROPERTY(QList<Poster> posters READ posters WRITE setPosters)
    Q_PROPERTY(QList<Poster> backdrops READ backdrops WRITE setBackdrops)
    Q_PROPERTY(bool watched READ watched WRITE setWatched)
    Q_PROPERTY(bool hasChanged READ hasChanged WRITE setChanged)
    Q_PROPERTY(TmdbId tmdbId READ tmdbId WRITE setTmdbId)

public:
    explicit Concert(QStringList files, QObject *parent = nullptr);
    ~Concert() = default;

    ConcertController *controller() const;

    void clear();
    void clear(QList<ConcertScraperInfos> infos);

    virtual QString name() const;
    virtual QString artist() const;
    virtual QString album() const;
    virtual QString overview() const;
    virtual qreal rating() const;
    virtual QDate released() const;
    virtual QString tagline() const;
    virtual std::chrono::minutes runtime() const;
    virtual QString certification() const;
    virtual QStringList genres() const;
    virtual QStringList tags() const;
    virtual QList<QString *> genresPointer();
    virtual QUrl trailer() const;
    virtual QStringList files() const;
    virtual QString folderName() const;
    virtual int playcount() const;
    virtual QDateTime lastPlayed() const;
    virtual QList<Poster> posters() const;
    virtual QList<Poster> backdrops() const;
    virtual bool watched() const;
    virtual int concertId() const;
    virtual bool downloadsInProgress() const;
    virtual int downloadsSize() const;
    virtual bool inSeparateFolder() const;
    virtual int mediaCenterId() const;
    virtual TmdbId tmdbId() const;
    virtual ImdbId imdbId() const;
    virtual StreamDetails *streamDetails() const;
    virtual bool streamDetailsLoaded() const;
    virtual QString nfoContent() const;
    virtual int databaseId() const;
    virtual bool syncNeeded() const;

    bool hasChanged() const;

    void setFiles(QStringList files);
    void setName(QString name);
    void setArtist(QString artist);
    void setAlbum(QString album);
    void setOverview(QString overview);
    void setRating(qreal rating);
    void setReleased(QDate released);
    void setTagline(QString tagline);
    void setRuntime(std::chrono::minutes runtime);
    void setCertification(QString certification);
    void setTrailer(QUrl trailer);
    void addGenre(QString genre);
    void addTag(QString tag);
    void setPlayCount(int playcount);
    void setLastPlayed(QDateTime lastPlayed);
    void setPosters(QList<Poster> posters);
    void setPoster(int index, Poster poster);
    void addPoster(Poster poster);
    void setBackdrops(QList<Poster> backdrops);
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
    QList<ExtraFanart> extraFanarts(MediaCenterInterface *mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QList<QByteArray> extraFanartImagesToAdd();
    void addExtraFanart(QByteArray fanart);
    void removeExtraFanart(QByteArray fanart);
    void removeExtraFanart(QString file);
    void clearExtraFanartData();

    void clearImages();
    void removeImage(ImageType type);
    QList<ImageType> imagesToRemove() const;

    QByteArray image(ImageType imageType);
    bool imageHasChanged(ImageType imageType);
    void setImage(ImageType imageType, QByteArray image);
    void setHasImage(ImageType imageType, bool has);
    bool hasImage(ImageType imageType);
    bool hasExtraFanarts() const;
    void setHasExtraFanarts(bool has);

    void scraperLoadDone();
    QList<ConcertScraperInfos> infosToLoad();
    void setLoadsLeft(QList<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);

    void setDiscType(DiscType type);
    DiscType discType() const;

    static bool lessThan(Concert *a, Concert *b);
    static QList<ImageType> imageTypes();

signals:
    void sigChanged(Concert *);

private:
    ConcertController *m_controller;
    QStringList m_files;
    QString m_folderName;
    QString m_name;
    QString m_artist;
    QString m_album;
    QString m_overview;
    qreal m_rating;
    QDate m_released;
    QString m_tagline;
    std::chrono::minutes m_runtime;
    QString m_certification;
    QStringList m_genres;
    QStringList m_tags;
    QUrl m_trailer;
    int m_playcount;
    QDateTime m_lastPlayed;
    QList<Poster> m_posters;
    QList<Poster> m_backdrops;
    int m_concertId;
    int m_downloadsSize;
    bool m_watched;
    bool m_hasChanged;
    bool m_downloadsInProgress;
    bool m_inSeparateFolder;
    int m_mediaCenterId;
    TmdbId m_tmdbId;
    ImdbId m_imdbId;
    QList<ConcertScraperInfos> m_infosToLoad;
    bool m_streamDetailsLoaded;
    StreamDetails *m_streamDetails;
    QString m_nfoContent;
    int m_databaseId;
    bool m_syncNeeded;
    QList<ScraperData> m_loadsLeft;
    QMutex m_loadMutex;
    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    bool m_hasExtraFanarts;

    QMap<ImageType, QByteArray> m_images;
    QMap<ImageType, bool> m_hasImageChanged;
    QList<QByteArray> m_extraFanartImagesToAdd;
    QList<ImageType> m_imagesToRemove;
    QMap<ImageType, bool> m_hasImage;
};

#endif // CONCERT_H
