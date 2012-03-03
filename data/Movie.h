#ifndef MOVIE_H
#define MOVIE_H

#include <QDate>
#include <QDebug>
#include <QPixmap>
#include <QObject>
#include <QStringList>
#include <QUrl>

#include "Globals.h"
#include "data/MediaCenterInterface.h"
#include "data/ScraperInterface.h"

class MediaCenterInterface;
class ScraperInterface;

class Movie : public QObject
{
    Q_OBJECT
public:
    explicit Movie(QStringList files, QObject *parent = 0);
    ~Movie();

    void clear();

    QString name() const;
    QString originalName() const;
    QString overview() const;
    qreal rating() const;
    QDate released() const;
    QString tagline() const;
    int runtime() const;
    QString certification() const;
    QStringList genres() const;
    QStringList countries() const;
    QStringList studios() const;
    QUrl trailer() const;
    QList<Actor> actors() const;
    QList<Actor*> actorsPointer();
    QStringList files() const;
    QString folderName() const;
    int playcount() const;
    QDateTime lastPlayed() const;
    QString id() const;
    QString set() const;
    QList<Poster> posters() const;
    QList<Poster> backdrops() const;
    QImage *posterImage();
    QImage *backdropImage();
    bool infoLoaded() const;
    bool posterImageChanged() const;
    bool backdropImageChanged() const;

    void setName(QString name);
    void setOriginalName(QString originalName);
    void setOverview(QString overview);
    void setRating(qreal rating);
    void setReleased(QDate released);
    void setTagline(QString tagline);
    void setRuntime(int runtime);
    void setCertification(QString certification);
    void setGenres(QStringList genres);
    void setCountries(QStringList countries);
    void setStudios(QStringList studios);
    void addStudio(QString studio);
    void setTrailer(QUrl trailer);
    void setActors(QList<Actor> actors);
    void addActor(Actor actor);
    void addGenre(QString genre);
    void addCountry(QString country);
    void setPlayCount(int playcount);
    void setLastPlayed(QDateTime lastPlayed);
    void setId(QString id);
    void setSet(QString set);
    void setPosters(QList<Poster> posters);
    void setPoster(int index, Poster poster);
    void addPoster(Poster poster);
    void setBackdrops(QList<Poster> backdrops);
    void setBackdrop(int index, Poster backdrop);
    void addBackdrop(Poster backdrop);
    void setPosterImage(QImage poster);
    void setBackdropImage(QImage backdrop);

    bool saveData(MediaCenterInterface *mediaCenterInterface);
    bool loadData(MediaCenterInterface *mediaCenterInterface);
    void loadData(QString id, ScraperInterface *scraperInterface);
    void loadImages(MediaCenterInterface *mediaCenterInterface);
    
    void scraperLoadDone();

signals:
    void loaded();
    
private:
    QStringList m_files;
    QString m_folderName;
    QString m_name;
    QString m_originalName;
    QString m_overview;
    qreal m_rating;
    QDate m_released;
    QString m_tagline;
    int m_runtime;
    QString m_certification;
    QStringList m_genres;
    QStringList m_countries;
    QStringList m_studios;
    QUrl m_trailer;
    QList<Actor> m_actors;
    int m_playcount;
    QDateTime m_lastPlayed;
    QString m_id;
    QString m_set;
    QList<Poster> m_posters;
    QList<Poster> m_backdrops;
    QImage m_posterImage;
    QImage m_backdropImage;
    bool m_posterImageChanged;
    bool m_backdropImageChanged;
    bool m_infoLoaded;
};

QDebug operator<<(QDebug dbg, const Movie &movie);
QDebug operator<<(QDebug dbg, const Movie *movie);

#endif // MOVIE_H
