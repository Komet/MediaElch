#pragma once

#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Rating.h"
#include "data/ResumeTime.h"
#include "data/Subtitle.h"
#include "data/TmdbId.h"
#include "data/WikidataId.h"
#include "data/movie/MovieController.h"
#include "data/movie/MovieCrew.h"
#include "data/movie/MovieImages.h"
#include "data/movie/MovieSet.h"
#include "database/DatabaseId.h"
#include "globals/Globals.h"
#include "media/StreamDetails.h"
#include "utils/Meta.h"

#include <QDate>
#include <QDebug>
#include <QMap>
#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QVector>
#include <chrono>

class MediaCenterInterface;

/// \brief The Movie class represents a single movie
class Movie : public QObject
{
    Q_OBJECT

public:
    /// \param files List of files for this movie
    explicit Movie(QStringList files = {}, QObject* parent = nullptr);
    ~Movie() override = default;

    MovieController* controller() const;

    void clear();
    void clear(QSet<MovieScraperInfo> infos);
    void clearImages();

    struct Exporter;
    /// \brief Write all fields to the given exporter.
    /// \see Movie::Exporter
    void exportTo(Exporter& exporter) const;

    ELCH_NODISCARD QString title() const;
    ELCH_NODISCARD QString sortTitle() const;
    ELCH_NODISCARD QString originalTitle() const;

    ELCH_NODISCARD MovieImages& images();
    ELCH_NODISCARD const MovieImages& constImages() const;
    ELCH_NODISCARD QString overview() const;
    ELCH_NODISCARD Ratings& ratings();
    ELCH_NODISCARD const Ratings& ratings() const;
    ELCH_NODISCARD double userRating() const;
    ELCH_NODISCARD int top250() const;
    ELCH_NODISCARD QDate released() const;
    ELCH_NODISCARD QString tagline() const;
    ELCH_NODISCARD QString outline() const;
    ELCH_NODISCARD std::chrono::minutes runtime() const;
    ELCH_NODISCARD Certification certification() const;
    ELCH_NODISCARD QString writer() const;
    ELCH_NODISCARD QString director() const;
    ELCH_NODISCARD QStringList genres() const;
    ELCH_NODISCARD QVector<QString*> genresPointer();
    ELCH_NODISCARD QStringList countries() const;
    ELCH_NODISCARD QVector<QString*> countriesPointer();
    ELCH_NODISCARD QStringList studios() const;
    ELCH_NODISCARD QStringList tags() const;
    ELCH_NODISCARD QVector<QString*> studiosPointer();
    ELCH_NODISCARD QUrl trailer() const;
    ELCH_NODISCARD QStringList tvShowLinks() const;

    ELCH_NODISCARD const Actors& actors() const;
    ELCH_NODISCARD Actors& actors();

    ELCH_NODISCARD QMap<QString, QString> idsForScrapers();

    ELCH_NODISCARD ImdbId imdbId() const;
    void setImdbId(ImdbId imdbId);
    ELCH_NODISCARD TmdbId tmdbId() const;
    void setTmdbId(TmdbId tmdbId);
    ELCH_NODISCARD WikidataId wikidataId() const;
    void setWikidataId(WikidataId wikidataId);

    ELCH_NODISCARD const mediaelch::FileList& files() const;
    ELCH_NODISCARD QString folderName() const;
    ELCH_NODISCARD int playCount() const;
    ELCH_NODISCARD QDateTime lastPlayed() const;
    ELCH_NODISCARD MovieSet set() const;
    ELCH_NODISCARD bool watched() const;
    ELCH_NODISCARD int movieId() const;
    ELCH_NODISCARD bool inSeparateFolder() const;
    ELCH_NODISCARD int mediaCenterId() const;
    ELCH_NODISCARD StreamDetails* streamDetails();
    ELCH_NODISCARD bool streamDetailsLoaded() const;
    ELCH_NODISCARD QDateTime fileLastModified() const;
    ELCH_NODISCARD QString nfoContent() const;
    ELCH_NODISCARD mediaelch::DatabaseId databaseId() const;
    ELCH_NODISCARD bool syncNeeded() const;
    ELCH_NODISCARD bool hasLocalTrailer() const;
    ELCH_NODISCARD QDateTime dateAdded() const;
    ELCH_NODISCARD mediaelch::ResumeTime resumeTime() const;
    ELCH_NODISCARD bool hasValidImdbId() const;
    ELCH_NODISCARD bool hasImage(ImageType imageType) const;

    ELCH_NODISCARD bool hasChanged() const;
    ELCH_NODISCARD QString localTrailerFileName() const;

    void setFiles(const mediaelch::FileList& files);
    void setTitle(QString title);
    void setSortTitle(QString sortTitle);
    void setOriginalTitle(QString originalTitle);
    void setOverview(QString overview);
    void setTop250(int top250);
    void setReleased(QDate released);
    void setTagline(QString tagline);
    void setOutline(QString outline);
    void setRuntime(std::chrono::minutes runtime);
    void setCertification(Certification certification);
    void setWriter(QString writer);
    void setDirector(QString director);
    void addStudio(QString studio);
    void addTag(QString tag);
    void setTrailer(QUrl trailer);
    void setTvShowLinks(QStringList tvShowLinks);
    void setActors(QVector<Actor> actors);
    void addActor(Actor actor);
    void addGenre(QString genre);
    void addCountry(QString country);
    void setPlayCount(int playCount);
    void setLastPlayed(QDateTime lastPlayed);
    void setSet(MovieSet set);
    void setUserRating(double rating);
    void setChanged(bool changed);
    void setDownloadsInProgress(bool inProgress);
    void setDownloadsSize(int downloadsSize);
    void setInSeparateFolder(bool inSepFolder);
    void setMediaCenterId(int mediaCenterId);
    void setFileLastModified(QDateTime modified);
    void setNfoContent(QString content);
    void setDatabaseId(mediaelch::DatabaseId id);
    void setSyncNeeded(bool syncNeeded);
    void setDateAdded(QDateTime date);
    void setResumeTime(mediaelch::ResumeTime time);

    void removeActor(Actor* actor);
    void removeCountry(QString* country);
    void removeCountry(QString country);
    void removeStudio(QString* studio);
    void removeStudio(QString studio);
    void removeGenre(QString* genre);
    void removeGenre(QString genre);
    void removeTag(QString tag);

    void setLabel(ColorLabel label);
    ELCH_NODISCARD ColorLabel label() const;

    ELCH_NODISCARD DiscType discType() const;
    void setDiscType(DiscType type);

    ELCH_NODISCARD static bool lessThan(Movie* a, Movie* b);
    ELCH_NODISCARD static QSet<ImageType> imageTypes();

    /// \brief Whether there are external subtitles or streamdetails subtitles.
    ELCH_NODISCARD bool hasSubtitles() const;
    QVector<Subtitle*> subtitles() const;
    void addSubtitle(Subtitle* subtitle, bool fromLoad = false);

    ELCH_NODISCARD bool isDuplicate(Movie* movie) const;

    ELCH_NODISCARD bool hasDuplicates() const;
    void setHasDuplicates(bool hasDuplicates);

    ELCH_NODISCARD MovieDuplicate duplicateProperties(Movie* movie) const;

public:
    /// \brief   Export interface for Movie::exportTo().
    /// \details Implement this interface and pass instances of it to Movie::exportTo()
    ///          to export the movie's data.  By using this Exporter, you ensure that
    ///          you will get notified of new fields (due to compilation errors).
    /// \todo    This structure does not export _everything_, for example m_nfoContent,
    ///          since that should not be part of a movie's data, but is more of an
    ///          implementation detail.  The Movie class needs some refactoring.
    struct Exporter
    {
        virtual void startExport() = 0;
        virtual void endExport() = 0;

        virtual void exportMovieId(int movieId) = 0;
        virtual void exportDatabaseId(mediaelch::DatabaseId databaseId) = 0;
        virtual void exportImdbId(const ImdbId& imdbId) = 0;
        virtual void exportTmdbId(const TmdbId& tmdbId) = 0;
        virtual void exportWikidataId(const WikidataId& wikidataId) = 0;
        virtual void exportMediaCenterId(int mediaCenterId) = 0;

        virtual void exportTitle(const QString& title) = 0;
        virtual void exportSortTitle(const QString& sortTitle) = 0;
        virtual void exportOriginalTitle(const QString& originalTitle) = 0;

        virtual void exportFiles(const mediaelch::FileList& files) = 0;
        virtual void exportMovieImages(const MovieImages& movieImages) = 0;
        virtual void exportFolderName(const QString& folderName) = 0;
        virtual void exportOverview(const QString& overview) = 0;
        virtual void exportRatings(const Ratings& ratings) = 0;
        virtual void exportUserRating(double userRating) = 0;
        virtual void exportImdbTop250(int imdbTop250) = 0;
        virtual void exportReleased(const QDate& released) = 0;
        virtual void exportTagline(const QString& tagline) = 0;
        virtual void exportOutline(const QString& outline) = 0;
        virtual void exportCrew(const MovieCrew& crew) = 0;
        virtual void exportRuntime(std::chrono::minutes runtime) = 0;
        virtual void exportCertification(const Certification& certification) = 0;
        virtual void exportGenres(const QStringList& genres) = 0;
        virtual void exportCountries(const QStringList& countries) = 0;
        virtual void exportStudios(const QStringList& studios) = 0;
        virtual void exportTags(const QStringList& tags) = 0;
        virtual void exportTrailer(const QUrl& trailer) = 0;
        virtual void exportTvShowLinks(const QStringList& tvShowLinks) = 0;
        virtual void exportPlayCount(int playCount) = 0;
        virtual void exportLastPlayed(const QDateTime& lastPlayed) = 0;
        virtual void exportMovieSet(const MovieSet& set) = 0;
        virtual void exportStreamDetails(const StreamDetails* streamDetails) = 0;
        virtual void exportFileLastModified(const QDateTime& fileLastModified) = 0;
        virtual void exportDateAdded(const QDateTime& dateAdded) = 0;
        virtual void exportDiscType(DiscType discType) = 0;
        virtual void exportLabel(const ColorLabel& label) = 0;
        virtual void exportSubtitles(const QVector<Subtitle*>& subtitles) = 0;
        virtual void exportResumeTime(mediaelch::ResumeTime resumeTime) = 0;
    };

signals:
    void sigChanged(Movie*);

private slots:
    void onSubtitleChanged();

private:
    MovieController* m_controller;
    mediaelch::FileList m_files;
    MovieImages m_movieImages;
    QString m_folderName;
    QString m_name;
    QString m_sortTitle;
    QString m_originalTitle;
    QString m_overview;
    Ratings m_ratings;
    double m_userRating = 0.0;
    int m_imdbTop250 = 0;
    QDate m_released;
    QString m_tagline;
    QString m_outline;
    MovieCrew m_crew;
    std::chrono::minutes m_runtime;
    Certification m_certification;
    QStringList m_genres;
    QStringList m_countries;
    QStringList m_studios;
    QStringList m_tags;
    QUrl m_trailer;
    QStringList m_tvShowLinks;
    int m_playCount = 0;
    QDateTime m_lastPlayed;
    ImdbId m_imdbId;
    TmdbId m_tmdbId;
    WikidataId m_wikidataId;
    MovieSet m_set;
    int m_movieId = -1;
    mediaelch::DatabaseId m_databaseId;
    int m_mediaCenterId = -1;
    bool m_hasChanged = false;
    bool m_inSeparateFolder = false;
    bool m_syncNeeded = false;
    bool m_hasDuplicates = false;
    StreamDetails* m_streamDetails;
    QDateTime m_fileLastModified;
    QString m_nfoContent;
    QDateTime m_dateAdded;
    DiscType m_discType;
    ColorLabel m_label;
    QVector<Subtitle*> m_subtitles;
    mediaelch::ResumeTime m_resumeTime;
};

Q_DECLARE_METATYPE(Movie*)

QDebug operator<<(QDebug dbg, const Movie& movie);
QDebug operator<<(QDebug dbg, const Movie* movie);
