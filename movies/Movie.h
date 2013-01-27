#ifndef MOVIE_H
#define MOVIE_H

#include <QDate>
#include <QDebug>
#include <QPixmap>
#include <QObject>
#include <QStringList>
#include <QUrl>

#include "globals/Globals.h"
#include "data/MediaCenterInterface.h"
#include "data/ScraperInterface.h"
#include "data/StreamDetails.h"
#include "movies/MovieController.h"

class MovieController;
class MediaCenterInterface;
class ScraperInterface;
class StreamDetails;

/**
 * @brief The Movie class
 * This class represents a single movie
 */
class Movie : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString sortTitle READ sortTitle WRITE setSortTitle)
    Q_PROPERTY(QString originalName READ originalName WRITE setOriginalName)
    Q_PROPERTY(QString overview READ overview WRITE setOverview)
    Q_PROPERTY(qreal rating READ rating WRITE setRating)
    Q_PROPERTY(int votes READ votes WRITE setVotes)
    Q_PROPERTY(int top250 READ top250 WRITE setTop250)
    Q_PROPERTY(QDate released READ released WRITE setReleased)
    Q_PROPERTY(QString tagline READ tagline WRITE setTagline)
    Q_PROPERTY(QString outline READ outline WRITE setOutline)
    Q_PROPERTY(int runtime READ runtime WRITE setRuntime)
    Q_PROPERTY(QString certification READ certification WRITE setCertification)
    Q_PROPERTY(QString writer READ writer WRITE setWriter)
    Q_PROPERTY(QString director READ director WRITE setDirector)
    Q_PROPERTY(QStringList genres READ genres WRITE setGenres)
    Q_PROPERTY(QStringList countries READ countries WRITE setCountries)
    Q_PROPERTY(QStringList studios READ studios WRITE setStudios)
    Q_PROPERTY(QUrl trailer READ trailer WRITE setTrailer)
    Q_PROPERTY(QList<Actor> actors READ actors WRITE setActors)
    Q_PROPERTY(int playcount READ playcount WRITE setPlayCount)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed WRITE setLastPlayed)
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QString set READ set WRITE setSet)
    Q_PROPERTY(QList<Poster> posters READ posters WRITE setPosters)
    Q_PROPERTY(QList<Poster> backdrops READ backdrops WRITE setBackdrops)
    Q_PROPERTY(bool watched READ watched WRITE setWatched)
    Q_PROPERTY(bool hasChanged READ hasChanged WRITE setChanged)
    Q_PROPERTY(QString tmdbId READ tmdbId WRITE setTmdbId)

public:
    explicit Movie(QStringList files, QObject *parent = 0);
    ~Movie();

    MovieController *controller();

    void clear();
    void clear(QList<int> infos);

    QString name() const;
    QString sortTitle() const;
    QString originalName() const;
    QString overview() const;
    qreal rating() const;
    int votes() const;
    int top250() const;
    QDate released() const;
    QString tagline() const;
    QString outline() const;
    int runtime() const;
    QString certification() const;
    QString writer() const;
    QString director() const;
    QStringList genres() const;
    QList<QString*> genresPointer();
    QStringList countries() const;
    QList<QString*> countriesPointer();
    QStringList studios() const;
    QStringList tags() const;
    QList<QString*> studiosPointer();
    QUrl trailer() const;
    QList<Actor> actors() const;
    QList<Actor*> actorsPointer();
    QStringList files() const;
    QString folderName() const;
    int playcount() const;
    QDateTime lastPlayed() const;
    QString id() const;
    QString tmdbId() const;
    QString set() const;
    QList<Poster> posters() const;
    QList<Poster> backdrops() const;
    QImage *posterImage();
    QImage *backdropImage();
    QImage *logoImage();
    QImage *clearArtImage();
    QImage *cdArtImage();
    bool posterImageChanged() const;
    bool backdropImageChanged() const;
    bool logoImageChanged() const;
    bool clearArtImageChanged() const;
    bool cdArtImageChanged() const;
    bool watched() const;
    int movieId() const;
    bool inSeparateFolder() const;
    int mediaCenterId() const;
    int numPrimaryLangPosters() const;
    bool hasPoster() const;
    bool hasBackdrop() const;
    bool hasLogo() const;
    bool hasClearArt() const;
    bool hasCdArt() const;
    StreamDetails *streamDetails();
    bool streamDetailsLoaded() const;
    QDateTime fileLastModified() const;
    QString nfoContent() const;
    int databaseId() const;
    bool syncNeeded() const;
    bool hasLocalTrailer() const;
    QList<ExtraFanart> extraFanarts(MediaCenterInterface *mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QList<QImage> extraFanartImagesToAdd();

    bool hasChanged() const;

    void setName(QString name);
    void setSortTitle(QString sortTitle);
    void setOriginalName(QString originalName);
    void setOverview(QString overview);
    void setRating(qreal rating);
    void setVotes(int votes);
    void setTop250(int top250);
    void setReleased(QDate released);
    void setTagline(QString tagline);
    void setOutline(QString outline);
    void setRuntime(int runtime);
    void setCertification(QString certification);
    void setWriter(QString writer);
    void setDirector(QString director);
    void setGenres(QStringList genres);
    void setCountries(QStringList countries);
    void setStudios(QStringList studios);
    void addStudio(QString studio);
    void addTag(QString tag);
    void setTrailer(QUrl trailer);
    void setActors(QList<Actor> actors);
    void addActor(Actor actor);
    void addGenre(QString genre);
    void addCountry(QString country);
    void setPlayCount(int playcount);
    void setLastPlayed(QDateTime lastPlayed);
    void setId(QString id);
    void setTmdbId(QString id);
    void setSet(QString set);
    void setPosters(QList<Poster> posters);
    void setPoster(int index, Poster poster);
    void addPoster(Poster poster,bool primaryLang = false);
    void setBackdrops(QList<Poster> backdrops);
    void setBackdrop(int index, Poster backdrop);
    void addBackdrop(Poster backdrop);
    void setPosterImage(QImage poster);
    void setBackdropImage(QImage backdrop);
    void setLogoImage(QImage img);
    void setClearArtImage(QImage img);
    void setCdArtImage(QImage img);
    void setWatched(bool watched);
    void setChanged(bool changed);
    void setDownloadsInProgress(bool inProgress);
    void setDownloadsSize(int downloadsSize);
    void setInSeparateFolder(bool inSepFolder);
    void setMediaCenterId(int mediaCenterId);
    void setNumPrimaryLangPosters(int numberPrimaryLangPosters);
    void setHasPoster(bool has);
    void setHasBackdrop(bool has);
    void setHasLogo(bool has);
    void setHasClearArt(bool has);
    void setHasCdArt(bool has);
    void setStreamDetailsLoaded(bool loaded);
    void setFileLastModified(QDateTime modified);
    void setNfoContent(QString content);
    void setDatabaseId(int id);
    void setSyncNeeded(bool syncNeeded);
    void addExtraFanart(QImage fanart);
    void removeExtraFanart(QImage fanart);
    void removeExtraFanart(QString file);
    void clearExtraFanartData();

    void removeActor(Actor *actor);
    void removeCountry(QString *country);
    void removeCountry(QString country);
    void removeStudio(QString *studio);
    void removeStudio(QString studio);
    void removeGenre(QString *genre);
    void removeGenre(QString genre);
    void removeTag(QString tag);

    void clearImages();

signals:
    void sigChanged(Movie*);

private:
    MovieController *m_controller;
    QStringList m_files;
    QString m_folderName;
    QString m_name;
    QString m_sortTitle;
    QString m_originalName;
    QString m_overview;
    qreal m_rating;
    int m_votes;
    int m_top250;
    QDate m_released;
    QString m_tagline;
    QString m_outline;
    int m_runtime;
    QString m_certification;
    QString m_writer;
    QString m_director;
    QStringList m_genres;
    QStringList m_countries;
    QStringList m_studios;
    QStringList m_tags;
    QUrl m_trailer;
    QList<Actor> m_actors;
    int m_playcount;
    QDateTime m_lastPlayed;
    QString m_id;
    QString m_tmdbId;
    QString m_set;
    QList<Poster> m_posters;
    QList<Poster> m_backdrops;
    QImage m_posterImage;
    QImage m_backdropImage;
    QImage m_logoImage;
    QImage m_clearArtImage;
    QImage m_cdArtImage;
    QList<QImage> m_extraFanartImagesToAdd;
    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    bool m_posterImageChanged;
    bool m_backdropImageChanged;
    bool m_logoImageChanged;
    bool m_clearArtImageChanged;
    bool m_cdArtImageChanged;
    bool m_infoLoaded;
    bool m_infoFromNfoLoaded;
    bool m_imagesLoaded;
    bool m_watched;
    bool m_hasChanged;
    int m_movieId;
    bool m_inSeparateFolder;
    int m_mediaCenterId;
    int m_numPrimaryLangPosters;
    bool m_hasPoster;
    bool m_hasBackdrop;
    bool m_hasLogo;
    bool m_hasClearArt;
    bool m_hasCdArt;
    bool m_syncNeeded;
    bool m_streamDetailsLoaded;
    StreamDetails *m_streamDetails;
    QDateTime m_fileLastModified;
    QString m_nfoContent;
    int m_databaseId;
};

Q_DECLARE_METATYPE(Movie*)

QDebug operator<<(QDebug dbg, const Movie &movie);
QDebug operator<<(QDebug dbg, const Movie *movie);

#endif // MOVIE_H
