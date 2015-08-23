#ifndef ARTIST_H
#define ARTIST_H

#include <QObject>
#include "globals/Globals.h"
#include "MusicModelItem.h"
#include "ArtistController.h"

class ArtistController;
class MusicModelItem;

class Artist : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MusicModelItem* modelItem READ modelItem NOTIFY modelItemChanged)
public:
    explicit Artist(QString path, QObject *parent = 0);
    ~Artist();

    QString path() const;
    void setPath(const QString &path);

    QString name() const;
    void setName(const QString &name);

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

    QString yearsActive() const;
    void setYearsActive(const QString &yearsActive);

    QString formed() const;
    void setFormed(const QString &formed);

    QString born() const;
    void setBorn(const QString &born);

    QString died() const;
    void setDied(const QString &died);

    QString disbanded() const;
    void setDisbanded(const QString &disbanded);

    QString biography() const;
    void setBiography(const QString &biography);

    QList<Poster> images(int imageType) const;
    void addImage(int imageType, Poster image);

    QByteArray rawImage(int imageType);
    void setRawImage(int imageType, QByteArray image);
    void removeImage(int imageType);
    void clearImages();

    bool hasChanged() const;
    void setHasChanged(bool hasChanged);

    void clear();
    void clear(QList<int> infos);

    MusicModelItem *modelItem() const;
    void setModelItem(MusicModelItem *modelItem);

    QString nfoContent() const;
    void setNfoContent(const QString &nfoContent);

    static QList<int> imageTypes();

    QList<int> imagesToRemove() const;
    void setImagesToRemove(const QList<int> &imagesToRemove);

    int databaseId() const;
    void setDatabaseId(int databaseId);

    ArtistController *controller() const;
    void setController(ArtistController *controller);

    QString mbId() const;
    void setMbId(const QString &mbId);

    QList<Album*> albums() const;
    void setAlbums(const QList<Album*> &albums);
    void addAlbum(Album *album);

    QString allMusicId() const;
    void setAllMusicId(const QString &allMusicId);

    QList<ExtraFanart> extraFanarts(MediaCenterInterface *mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QList<QByteArray> extraFanartImagesToAdd();
    void addExtraFanart(QByteArray fanart);
    void removeExtraFanart(QByteArray fanart);
    void removeExtraFanart(QString file);
    void clearExtraFanartData();

    void setDiscographyAlbums(QList<DiscographyAlbum> albums);
    void addDiscographyAlbum(DiscographyAlbum album);
    void removeDiscographyAlbum(DiscographyAlbum *album);
    QList<DiscographyAlbum> discographyAlbums() const;
    QList<DiscographyAlbum*> discographyAlbumsPointer();

signals:
    void sigChanged(Artist*);
    void modelItemChanged();

private:
    QString m_path;
    QString m_name;
    QStringList m_genres;
    QStringList m_styles;
    QStringList m_moods;
    QString m_yearsActive;
    QString m_formed;
    QString m_biography;
    QString m_born;
    QString m_died;
    QString m_disbanded;
    bool m_hasChanged;
    QMap<int, QList<Poster> > m_images;
    QMap<int, QByteArray> m_rawImages;
    QList<int> m_imagesToRemove;
    MusicModelItem *m_modelItem;
    QString m_nfoContent;
    int m_databaseId;
    ArtistController *m_controller;
    QString m_mbId;
    QString m_allMusicId;
    QList<Album*> m_albums;
    QList<DiscographyAlbum> m_discography;

    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    QList<QByteArray> m_extraFanartImagesToAdd;
};

#endif // ARTIST_H
