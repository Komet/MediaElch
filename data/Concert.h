#ifndef CONCERT_H
#define CONCERT_H

#include <QDate>
#include <QDebug>
#include <QPixmap>
#include <QObject>
#include <QStringList>
#include <QUrl>

#include "globals/Globals.h"
#include "data/MediaCenterInterface.h"
#include "data/ConcertScraperInterface.h"

class MediaCenterInterface;
class ConcertScraperInterface;

/**
 * @brief The Concert class
 * This class represents a single concert
 */
class Concert : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(qreal rating READ rating WRITE setRating)
    Q_PROPERTY(QDate released READ released WRITE setReleased)
    Q_PROPERTY(QString overview READ overview WRITE setOverview)
    Q_PROPERTY(QString tagline READ tagline WRITE setTagline)
    Q_PROPERTY(int runtime READ runtime WRITE setRuntime)
    Q_PROPERTY(QString certification READ certification WRITE setCertification)
    Q_PROPERTY(int playcount READ playcount WRITE setPlayCount)
    Q_PROPERTY(QDateTime lastPlayed READ lastPlayed WRITE setLastPlayed)
    Q_PROPERTY(QStringList genres READ genres WRITE setGenres)
    Q_PROPERTY(QUrl trailer READ trailer WRITE setTrailer)
    Q_PROPERTY(QList<Poster> posters READ posters WRITE setPosters)
    Q_PROPERTY(QList<Poster> backdrops READ backdrops WRITE setBackdrops)
    Q_PROPERTY(bool watched READ watched WRITE setWatched)
    Q_PROPERTY(bool hasChanged READ hasChanged WRITE setChanged)

public:
    explicit Concert(QStringList files, QObject *parent = 0);
    ~Concert();

    void clear();
    void clear(QList<int> infos);

    QString name() const;
    QString overview() const;
    qreal rating() const;
    QDate released() const;
    QString tagline() const;
    int runtime() const;
    QString certification() const;
    QStringList genres() const;
    QList<QString*> genresPointer();
    QUrl trailer() const;
    QStringList files() const;
    QString folderName() const;
    int playcount() const;
    QDateTime lastPlayed() const;
    QList<Poster> posters() const;
    QList<Poster> backdrops() const;
    QImage *posterImage();
    QImage *backdropImage();
    bool infoLoaded() const;
    bool posterImageChanged() const;
    bool backdropImageChanged() const;
    bool watched() const;
    int concertId() const;
    bool downloadsInProgress() const;
    int downloadsSize() const;
    bool inSeparateFolder() const;
    int mediaCenterId() const;

    bool hasChanged() const;

    void setName(QString name);
    void setOverview(QString overview);
    void setRating(qreal rating);
    void setReleased(QDate released);
    void setTagline(QString tagline);
    void setRuntime(int runtime);
    void setCertification(QString certification);
    void setGenres(QStringList genres);
    void setTrailer(QUrl trailer);
    void addGenre(QString genre);
    void setPlayCount(int playcount);
    void setLastPlayed(QDateTime lastPlayed);
    void setPosters(QList<Poster> posters);
    void setPoster(int index, Poster poster);
    void addPoster(Poster poster);
    void setBackdrops(QList<Poster> backdrops);
    void setBackdrop(int index, Poster backdrop);
    void addBackdrop(Poster backdrop);
    void setPosterImage(QImage poster);
    void setBackdropImage(QImage backdrop);
    void setWatched(bool watched);
    void setChanged(bool changed);
    void setDownloadsInProgress(bool inProgress);
    void setDownloadsSize(int downloadsSize);
    void setInSeparateFolder(bool inSepFolder);
    void setMediaCenterId(int mediaCenterId);

    void removeGenre(QString *genre);

    bool saveData(MediaCenterInterface *mediaCenterInterface);
    bool loadData(MediaCenterInterface *mediaCenterInterface, bool force = false);
    void loadData(QString id, ConcertScraperInterface *scraperInterface, QList<int> infos);
    void loadImages(MediaCenterInterface *mediaCenterInterface, bool force = false);

    void scraperLoadDone();

signals:
    void loaded(Concert*);
    void sigChanged(Concert*);

private:
    QStringList m_files;
    QString m_folderName;
    QString m_name;
    QString m_overview;
    qreal m_rating;
    QDate m_released;
    QString m_tagline;
    int m_runtime;
    QString m_certification;
    QStringList m_genres;
    QUrl m_trailer;
    int m_playcount;
    QDateTime m_lastPlayed;
    QList<Poster> m_posters;
    QList<Poster> m_backdrops;
    QImage m_posterImage;
    QImage m_backdropImage;
    bool m_posterImageChanged;
    bool m_backdropImageChanged;
    bool m_infoLoaded;
    bool m_imagesLoaded;
    bool m_watched;
    bool m_hasChanged;
    int m_concertId;
    bool m_downloadsInProgress;
    int m_downloadsSize;
    bool m_inSeparateFolder;
    int m_mediaCenterId;
};

#endif // CONCERT_H
