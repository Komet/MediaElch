#pragma once

#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Rating.h"
#include "data/ResumeTime.h"
#include "data/StreamDetails.h"
#include "data/Subtitle.h"
#include "data/TmdbId.h"
#include "globals/Globals.h"
#include "movies/MovieController.h"
#include "movies/MovieCrew.h"
#include "movies/MovieImages.h"
#include "movies/MovieSet.h"

#include <QDate>
#include <QDebug>
#include <QObject>
#include <QPixmap>
#include <QStringList>
#include <QUrl>
#include <QVector>
#include <chrono>

class MediaCenterInterface;

/**
 * \brief The Movie class
 * This class represents a single movie
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

    QString name() const;
    QString sortTitle() const;
    QString originalName() const;
    MovieImages& images();
    const MovieImages& constImages() const;
    QString overview() const;
    QVector<Rating>& ratings();
    const QVector<Rating>& ratings() const;
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
    QVector<const Actor*> actors() const;
    QVector<Actor*> actors();
    const mediaelch::FileList& files() const;
    QString folderName() const;
    int playcount() const;
    QDateTime lastPlayed() const;
    ImdbId imdbId() const;
    TmdbId tmdbId() const;
    MovieSet set() const;
    bool watched() const;
    int movieId() const;
    bool inSeparateFolder() const;
    int mediaCenterId() const;
    StreamDetails* streamDetails();
    bool streamDetailsLoaded() const;
    QDateTime fileLastModified() const;
    QString nfoContent() const;
    int databaseId() const;
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
    void setActors(QVector<Actor> actors);
    void addActor(Actor actor);
    void addGenre(QString genre);
    void addCountry(QString country);
    void setPlayCount(int playcount);
    void setLastPlayed(QDateTime lastPlayed);
    void setImdbId(ImdbId imdbId);
    void setTmdbId(TmdbId tmdbId);
    void setSet(MovieSet set);
    void setUserRating(double rating);
    void setChanged(bool changed);
    void setDownloadsInProgress(bool inProgress);
    void setDownloadsSize(int downloadsSize);
    void setInSeparateFolder(bool inSepFolder);
    void setMediaCenterId(int mediaCenterId);
    void setStreamDetailsLoaded(bool loaded);
    void setFileLastModified(QDateTime modified);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
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
    static QVector<ImageType> imageTypes();

    QVector<Subtitle*> subtitles() const;
    void setSubtitles(const QVector<Subtitle*>& subtitles);
    void addSubtitle(Subtitle* subtitle, bool fromLoad = false);

    bool isDuplicate(Movie* movie) const;

    bool hasDuplicates() const;
    void setHasDuplicates(bool hasDuplicates);

    MovieDuplicate duplicateProperties(Movie* movie) const;

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
    QVector<Rating> m_ratings;
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
    int m_playcount = 0;
    QDateTime m_lastPlayed;
    ImdbId m_imdbId;
    TmdbId m_tmdbId;
    MovieSet m_set;
    int m_movieId = -1;
    int m_databaseId = -1;
    int m_mediaCenterId = -1;
    bool m_hasChanged = false;
    bool m_inSeparateFolder = false;
    bool m_syncNeeded = false;
    bool m_streamDetailsLoaded = false;
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
