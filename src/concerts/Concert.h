#pragma once

#include "concerts/ConcertController.h"
#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Rating.h"
#include "data/TmdbId.h"
#include "file/Path.h"
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

namespace mediaelch {

/// POD for concert data
class ConcertPod
{
public:
    int databaseId{-1};
    int concertId{-1};
    int mediaCenterId{-1};
    TmdbId tmdbId;
    ImdbId imdbId;

    QString title;
    QString originalTitle;
    QString artist;
    QString album;
    QString overview;

    QVector<Rating> ratings;
    double userRating = 0.0;

    QDate releaseDate;
    QString tagline;
    std::chrono::minutes runtime{0};
    Certification certification;

    QStringList genres;
    QStringList tags;
    QUrl trailer;

    int playcount{0};
    QDateTime lastPlayed;

    QVector<Poster> posters;
    QVector<Poster> backdrops;
    QStringList extraFanarts;

    StreamDetails* streamDetails = nullptr;
    QMap<ImageType, QByteArray> images;
};

} // namespace mediaelch

/// \brief The Concert class.
/// This class represents a single concert.
/// It is more a controller than actually a POD.
class Concert final : public QObject
{
    Q_OBJECT

public:
    explicit Concert(const mediaelch::FileList& files = {}, QObject* parent = nullptr);
    ~Concert() = default;

    ConcertController* controller() const;

    void clear();
    void clear(QSet<ConcertScraperInfo> infos);

    QString title() const;
    QString originalTitle() const;
    QString artist() const;
    QString album() const;
    QString overview() const;
    QVector<Rating>& ratings();
    const QVector<Rating>& ratings() const;
    double userRating() const;
    QDate released() const;
    QString tagline() const;
    std::chrono::minutes runtime() const;
    Certification certification() const;
    QStringList genres() const;
    QStringList tags() const;
    QVector<QString*> genresPointer();
    QUrl trailer() const;
    const mediaelch::FileList& files() const;
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

    void setFiles(const mediaelch::FileList& files);
    void setTitle(QString title);
    void setOriginalTitle(QString title);
    void setArtist(QString artist);
    void setAlbum(QString album);
    void setOverview(QString overview);
    void setUserRating(double rating);
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
    void setChanged(bool changed);
    void setDownloadsInProgress(bool inProgress);
    void setDownloadsSize(int downloadsLeft);
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

    QByteArray image(ImageType imageType) const;
    bool imageHasChanged(ImageType imageType);
    void setImage(ImageType imageType, QByteArray image);
    void setHasImage(ImageType imageType, bool has);
    bool hasImage(ImageType imageType);
    bool hasExtraFanarts() const;
    void setHasExtraFanarts(bool has);

    void scraperLoadDone();
    QSet<ConcertScraperInfo> infosToLoad();
    void setLoadsLeft(QVector<ScraperData> loadsLeft);
    void removeFromLoadsLeft(ScraperData load);

    void setDiscType(DiscType type);
    DiscType discType() const;

    static bool lessThan(Concert* a, Concert* b);
    static QVector<ImageType> imageTypes();

signals:
    void sigChanged(Concert*);

private:
    mediaelch::ConcertPod m_concert;

    ConcertController* m_controller;
    mediaelch::FileList m_files;
    QString m_folderName;

    int m_downloadsSize = 0;
    bool m_hasChanged;
    bool m_downloadsInProgress = false;
    bool m_inSeparateFolder;
    QSet<ConcertScraperInfo> m_infosToLoad;
    bool m_streamDetailsLoaded;
    QString m_nfoContent;
    bool m_syncNeeded;
    QVector<ScraperData> m_loadsLeft;
    QMutex m_loadMutex;
    QStringList m_extraFanartsToRemove;
    bool m_hasExtraFanarts;

    QMap<ImageType, bool> m_hasImageChanged;
    QVector<QByteArray> m_extraFanartImagesToAdd;
    QVector<ImageType> m_imagesToRemove;
    QMap<ImageType, bool> m_hasImage;
};
