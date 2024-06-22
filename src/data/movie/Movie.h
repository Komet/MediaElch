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

#include <QDate>
#include <QDebug>
#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QVector>
#include <chrono>

class MediaCenterInterface;

/**
 * \brief The Movie class represents a single movie
 */
class Movie : public QObject
{
    Q_OBJECT

public:
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

    QString name() const;
    QString sortTitle() const;
    QString originalName() const;
    MovieImages& images();
    const MovieImages& constImages() const;
    QString overview() const;
    Ratings& ratings();
    const Ratings& ratings() const;
    double userRating() const;
    int top250() const;
    QDate released() const;
    QString tagline() const;
    QString outline() const;
    std::chrono::minutes runtime() const;
    Certification certification() const;
    QString writer() const;
    QString director() const;
    QStringList genres() const;
    QVector<QString*> genresPointer();
    QStringList countries() const;
    QVector<QString*> countriesPointer();
    QStringList studios() const;
    QStringList tags() const;
    QVector<QString*> studiosPointer();
    QUrl trailer() const;
    QStringList tvShowLinks() const;

    const Actors& actors() const;
    Actors& actors();

    const mediaelch::FileList& files() const;
    QString folderName() const;
    int playcount() const;
    QDateTime lastPlayed() const;
    ImdbId imdbId() const;
    TmdbId tmdbId() const;
    WikidataId wikidataId() const;
    MovieSet set() const;
    bool watched() const;
    int movieId() const;
    bool inSeparateFolder() const;
    int mediaCenterId() const;
    StreamDetails* streamDetails();
    bool streamDetailsLoaded() const;
    QDateTime fileLastModified() const;
    QString nfoContent() const;
    mediaelch::DatabaseId databaseId() const;
    bool syncNeeded() const;
    bool hasLocalTrailer() const;
    QDateTime dateAdded() const;
    mediaelch::ResumeTime resumeTime() const;
    bool hasValidImdbId() const;
    bool hasImage(ImageType imageType) const;

    bool hasChanged() const;
    QString localTrailerFileName() const;

    void setFiles(const mediaelch::FileList& files);
    void setName(QString name);
    void setSortTitle(QString sortTitle);
    void setOriginalName(QString originalName);
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
    void setPlayCount(int playcount);
    void setLastPlayed(QDateTime lastPlayed);
    void setImdbId(ImdbId imdbId);
    void setTmdbId(TmdbId tmdbId);
    void setWikidataId(WikidataId wikidataId);
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
    ColorLabel label() const;

    DiscType discType() const;
    void setDiscType(DiscType type);

    static bool lessThan(Movie* a, Movie* b);
    static QSet<ImageType> imageTypes();

    /// \brief Whether there are external subtitles or streamdetails subtitles.
    bool hasSubtitles() const;
    QVector<Subtitle*> subtitles() const;
    void addSubtitle(Subtitle* subtitle, bool fromLoad = false);

    bool isDuplicate(Movie* movie) const;

    bool hasDuplicates() const;
    void setHasDuplicates(bool hasDuplicates);

    MovieDuplicate duplicateProperties(Movie* movie) const;

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
        virtual void exportPlaycount(int playcount) = 0;
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
    QString m_originalName;
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
    int m_playcount = 0;
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
