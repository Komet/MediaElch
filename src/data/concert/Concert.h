#pragma once

#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Rating.h"
#include "data/TmdbId.h"
#include "data/concert/ConcertController.h"
#include "database/DatabaseId.h"
#include "globals/Globals.h"
#include "media/Path.h"

#include <QByteArray>
#include <QDate>
#include <QDebug>
#include <QMutex>
#include <QObject>
#include <QStringList>
#include <QUrl>
#include <chrono>

class MediaCenterInterface;
class StreamDetails;

namespace mediaelch {


/// \brief Concert Data
class ConcertData final
{
public:
    struct Exporter;

    /// \brief Write all fields to the given exporter.
    /// \see ConcertData::Exporter
    void exportTo(Exporter& exporter) const;

public:
    DatabaseId databaseId{-1};
    int mediaCenterId{-1};
    TmdbId tmdbId;
    ImdbId imdbId;

    QString title;
    QString originalTitle;
    QString artist;
    QString album;
    QString overview;

    Ratings ratings;
    double userRating = 0.0;

    QDate releaseDate;
    QString tagline;
    std::chrono::minutes runtime{0};
    Certification certification;

    QStringList genres;
    QStringList tags;
    QUrl trailer;

    int playCount{0};
    QDateTime lastPlayed;
    QDateTime lastModified;

    QVector<Poster> posters;
    QVector<Poster> backdrops;
    QStringList extraFanarts;

    StreamDetails* streamDetails = nullptr;
    QMap<ImageType, QByteArray> images;

    mediaelch::FileList files;

public:
    /// \brief   Export interface for ConcertData::exportTo().
    /// \details Implement this interface and pass instances of it to ConcertData::exportTo()
    ///          to export the concert's data.  By using this Exporter, you ensure that
    ///          you will get notified of new fields (due to compilation errors).
    struct Exporter
    {
        virtual void startExport() = 0;
        virtual void endExport() = 0;

        virtual void exportDatabaseId(DatabaseId databaseId) = 0;
        virtual void exportMediaCenterId(int mediaCenterId) = 0;
        virtual void exportTmdbId(const TmdbId& tmdbId) = 0;
        virtual void exportImdbId(const ImdbId& imdbId) = 0;

        virtual void exportTitle(const QString& title) = 0;
        virtual void exportOriginalTitle(const QString& originalTitle) = 0;
        virtual void exportArtist(const QString& artist) = 0;
        virtual void exportAlbum(const QString& album) = 0;
        virtual void exportOverview(const QString& overview) = 0;

        virtual void exportRatings(const Ratings& ratings) = 0;
        virtual void exportUserRating(double userRating) = 0;

        virtual void exportReleaseDate(const QDate& releaseDate) = 0;
        virtual void exportTagline(const QString& tagline) = 0;
        virtual void exportRuntime(const std::chrono::minutes& runtime) = 0;
        virtual void exportCertification(const Certification& certification) = 0;

        virtual void exportGenres(const QStringList& genres) = 0;
        virtual void exportTags(const QStringList& tags) = 0;
        virtual void exportTrailer(const QUrl& trailer) = 0;

        virtual void exportPlayCount(const int& playCount) = 0;
        virtual void exportLastPlayed(const QDateTime& lastPlayed) = 0;
        virtual void exportLastModified(const QDateTime& lastModified) = 0;

        virtual void exportPosters(const QVector<Poster>& posters) = 0;
        virtual void exportBackdrops(const QVector<Poster>& backdrops) = 0;
        virtual void exportExtraFanarts(const QStringList& extraFanarts) = 0;

        virtual void exportStreamDetails(const StreamDetails* streamDetails) = 0;
        virtual void exportImages(const QMap<ImageType, QByteArray>& images) = 0;

        virtual void exportFiles(const mediaelch::FileList& files) = 0;
    };
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

public:
    void clear();
    void clear(QSet<ConcertScraperInfo> infos);

    void exportTo(mediaelch::ConcertData::Exporter& exporter) const { m_concert.exportTo(exporter); };

public:
    QString title() const;
    QString originalTitle() const;
    QString artist() const;
    QString album() const;
    QString overview() const;
    Ratings& ratings();
    const Ratings& ratings() const;
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
    int playCount() const;
    QDateTime lastPlayed() const;
    QDateTime lastModified() const;
    QVector<Poster> posters() const;
    QVector<Poster> backdrops() const;
    bool watched() const;
    bool downloadsInProgress() const;
    int downloadsSize() const;
    bool inSeparateFolder() const;
    int mediaCenterId() const;
    TmdbId tmdbId() const;
    ImdbId imdbId() const;
    StreamDetails* streamDetails() const;
    bool streamDetailsLoaded() const;
    QString nfoContent() const;
    mediaelch::DatabaseId databaseId() const;
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
    void setPlayCount(int playCount);
    void setLastPlayed(QDateTime lastPlayed);
    void setLastModified(QDateTime lastModified);
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
    void setNfoContent(QString content);
    void setDatabaseId(mediaelch::DatabaseId id);
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
    QSet<ImageType> imagesToRemove() const;

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
    static QSet<ImageType> imageTypes();

signals:
    void sigChanged(Concert*);

private:
    mediaelch::ConcertData m_concert;

    ConcertController* m_controller;
    QString m_folderName;

    int m_downloadsSize = 0;
    QSet<ConcertScraperInfo> m_infosToLoad;
    QString m_nfoContent;
    QVector<ScraperData> m_loadsLeft;
    QMutex m_loadMutex;
    QStringList m_extraFanartsToRemove;

    QMap<ImageType, bool> m_hasImageChanged;
    QVector<QByteArray> m_extraFanartImagesToAdd;
    QSet<ImageType> m_imagesToRemove;
    QMap<ImageType, bool> m_hasImage;

    bool m_hasExtraFanarts{false};
    bool m_syncNeeded{false};
    bool m_hasChanged{false};
    bool m_downloadsInProgress{false};
    bool m_inSeparateFolder{false};
};
