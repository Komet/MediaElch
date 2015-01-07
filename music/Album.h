#ifndef ALBUM_H
#define ALBUM_H

#include <QObject>
#include "globals/Globals.h"
#include "MusicModelItem.h"
#include "Artist.h"
#include "AlbumController.h"

class AlbumController;
class Artist;
class MusicModelItem;

class Album : public QObject
{
    Q_OBJECT
public:
    explicit Album(QString path, QObject *parent = 0);
    ~Album();

    QString path() const;
    void setPath(const QString &path);

    bool hasChanged() const;
    void setHasChanged(bool hasChanged);

    QString title() const;
    void setTitle(const QString &title);

    QString artist() const;
    void setArtist(const QString &artist);

    QStringList genres() const;
    void setGenres(const QStringList &genres);
    void addGenre(const QString &genre);
    void removeGenre(const QString &genre);

    QStringList styles() const;
    void setStyles(const QStringList &styles);
    void addStyle(const QString &style);
    void removeStyle(const QString &style);

    QStringList moods() const;
    void setMoods(const QStringList &moods);
    void addMood(const QString &mood);
    void removeMood(const QString &mood);

    QString review() const;
    void setReview(const QString &review);

    QDate releaseDate() const;
    void setReleaseDate(const QDate &releaseDate);

    QString label() const;
    void setLabel(const QString &label);

    qreal rating() const;
    void setRating(const qreal &rating);

    int year() const;
    void setYear(int year);

    QList<Poster> images(int imageType) const;
    void addImage(int imageType, Poster image);

    QByteArray rawImage(int imageType);
    void setRawImage(int imageType, QByteArray image);
    void removeImage(int imageType);
    void clearImages();

    void clear();
    void clear(QList<int> infos);

    MusicModelItem *modelItem() const;
    void setModelItem(MusicModelItem *item);

    QString nfoContent() const;
    void setNfoContent(const QString &nfoContent);

    static QList<int> imageTypes();

    QList<int> imagesToRemove() const;
    void setImagesToRemove(const QList<int> &imagesToRemove);

    int databaseId() const;
    void setDatabaseId(int databaseId);

    Artist *artistObj() const;
    void setArtistObj(Artist *artistObj);

    AlbumController *controller() const;
    void setController(AlbumController *controller);

    QString mbId() const;
    void setMbId(const QString &mbId);

signals:
    void sigChanged(Album*);

private:
    QString m_path;
    bool m_hasChanged;
    QString m_title;
    QString m_artist;
    QStringList m_genres;
    QStringList m_styles;
    QStringList m_moods;
    QString m_review;
    QDate m_releaseDate;
    QString m_label;
    qreal m_rating;
    int m_year;
    QMap<int, QList<Poster> > m_images;
    QMap<int, QByteArray> m_rawImages;
    QList<int> m_imagesToRemove;
    MusicModelItem *m_modelItem;
    QString m_nfoContent;
    int m_databaseId;
    Artist *m_artistObj;
    AlbumController *m_controller;
    QString m_mbId;
};

#endif // ALBUM_H
