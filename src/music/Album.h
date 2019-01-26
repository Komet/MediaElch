#pragma once

#include "globals/Globals.h"
#include "image/ImageModel.h"
#include "image/ImageProxyModel.h"
#include "music/AlbumController.h"
#include "music/Artist.h"
#include "music/MusicModelItem.h"

#include <QObject>

class AlbumController;
class Artist;
class MusicModelItem;

class Album : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ImageModel *bookletModel READ bookletModel CONSTANT)
    Q_PROPERTY(ImageProxyModel *bookletProxyModel READ bookletProxyModel CONSTANT)
    Q_PROPERTY(Artist *artistObj READ artistObj NOTIFY artistObjChanged)
    Q_PROPERTY(MusicModelItem *modelItem READ modelItem NOTIFY modelItemChanged)

public:
    explicit Album(QString path, QObject *parent = nullptr);
    ~Album() override = default;

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

    QString releaseDate() const;
    void setReleaseDate(const QString &releaseDate);

    QString label() const;
    void setLabel(const QString &label);

    qreal rating() const;
    void setRating(const qreal &rating);

    int year() const;
    void setYear(int year);

    QList<Poster> images(ImageType imageType) const;
    void addImage(ImageType imageType, Poster image);

    QByteArray rawImage(ImageType imageType);
    void setRawImage(ImageType imageType, QByteArray image);
    void removeImage(ImageType imageType);
    void clearImages();

    void clear();
    void clear(QList<MusicScraperInfos> infos);

    MusicModelItem *modelItem() const;
    void setModelItem(MusicModelItem *item);

    QString nfoContent() const;
    void setNfoContent(const QString &nfoContent);

    static QList<ImageType> imageTypes();

    QList<ImageType> imagesToRemove() const;
    void setImagesToRemove(const QList<ImageType> &imagesToRemove);

    int databaseId() const;
    void setDatabaseId(int databaseId);

    Artist *artistObj() const;
    void setArtistObj(Artist *artistObj);

    AlbumController *controller() const;
    void setController(AlbumController *controller);

    QString mbReleaseGroupId() const;
    void setMbReleaseGroupId(const QString &mbReleaseGroupId);

    QString mbAlbumId() const;
    void setMbAlbumId(const QString &mbAlbumId);

    QString allMusicId() const;
    void setAllMusicId(const QString &allMusicId);

    ImageModel *bookletModel() const;
    ImageProxyModel *bookletProxyModel() const;

    void loadBooklets(MediaCenterInterface *mediaCenterInterface);

signals:
    void sigChanged(Album *);
    void artistObjChanged();
    void modelItemChanged();

private:
    QString m_path;
    bool m_hasChanged;
    QString m_title;
    QString m_artist;
    QStringList m_genres;
    QStringList m_styles;
    QStringList m_moods;
    QString m_review;
    QString m_releaseDate;
    QString m_label;
    qreal m_rating;
    int m_year;
    QMap<ImageType, QList<Poster>> m_images;
    QMap<ImageType, QByteArray> m_rawImages;
    QList<ImageType> m_imagesToRemove;
    MusicModelItem *m_modelItem;
    QString m_nfoContent;
    int m_databaseId;
    Artist *m_artistObj;
    AlbumController *m_controller;
    QString m_mbAlbumId;
    QString m_mbReleaseGroupId;
    QString m_allMusicId;
    ImageModel *m_bookletModel;
    ImageProxyModel *m_bookletProxyModel;
};
