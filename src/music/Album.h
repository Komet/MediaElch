#pragma once

#include "globals/Globals.h"
#include "globals/Poster.h"
#include "image/ImageModel.h"
#include "image/ImageProxyModel.h"
#include "music/AlbumController.h"
#include "music/AllMusicId.h"
#include "music/Artist.h"
#include "music/MusicBrainzId.h"
#include "music/MusicModelItem.h"

#include <QObject>

class AlbumController;
class Artist;
class MusicModelItem;

class Album : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ImageModel* bookletModel READ bookletModel CONSTANT)
    Q_PROPERTY(ImageProxyModel* bookletProxyModel READ bookletProxyModel CONSTANT)
    Q_PROPERTY(Artist* artistObj READ artistObj NOTIFY artistObjChanged)
    Q_PROPERTY(MusicModelItem* modelItem READ modelItem NOTIFY modelItemChanged)

public:
    explicit Album(mediaelch::DirectoryPath path = {}, QObject* parent = nullptr);
    ~Album() override = default;

    const mediaelch::DirectoryPath& path() const;
    void setPath(const mediaelch::DirectoryPath& path);

    bool hasChanged() const;
    void setHasChanged(bool hasChanged);

    QString title() const;
    void setTitle(const QString& title);

    QString artist() const;
    void setArtist(const QString& artist);

    QStringList genres() const;
    void setGenres(const QStringList& genres);
    void addGenre(const QString& genre);
    void removeGenre(const QString& genre);

    QStringList styles() const;
    void setStyles(const QStringList& styles);
    void addStyle(const QString& style);
    void removeStyle(const QString& style);

    QStringList moods() const;
    void setMoods(const QStringList& moods);
    void addMood(const QString& mood);
    void removeMood(const QString& mood);

    QString review() const;
    void setReview(const QString& review);

    QString releaseDate() const;
    void setReleaseDate(const QString& releaseDate);

    QString label() const;
    void setLabel(const QString& label);

    qreal rating() const;
    void setRating(const qreal& rating);

    int year() const;
    void setYear(int year);

    QVector<Poster> images(ImageType imageType) const;
    void addImage(ImageType imageType, Poster image);

    QByteArray rawImage(ImageType imageType);
    void setRawImage(ImageType imageType, QByteArray image);
    void removeImage(ImageType imageType);
    void clearImages();

    void clear();
    void clear(QSet<MusicScraperInfo> infos);

    MusicModelItem* modelItem() const;
    void setModelItem(MusicModelItem* item);

    QString nfoContent() const;
    void setNfoContent(const QString& nfoContent);

    static QVector<ImageType> imageTypes();

    QVector<ImageType> imagesToRemove() const;
    void setImagesToRemove(const QVector<ImageType>& imagesToRemove);

    int databaseId() const;
    void setDatabaseId(int databaseId);

    Artist* artistObj() const;
    void setArtistObj(Artist* artistObj);

    AlbumController* controller() const;
    void setController(AlbumController* controller);

    MusicBrainzId mbReleaseGroupId() const;
    void setMbReleaseGroupId(const MusicBrainzId& mbId);

    MusicBrainzId mbAlbumId() const;
    void setMbAlbumId(const MusicBrainzId& mbAlbumId);

    AllMusicId allMusicId() const;
    void setAllMusicId(const AllMusicId& allMusicId);

    ImageModel* bookletModel() const;
    ImageProxyModel* bookletProxyModel() const;

    void loadBooklets(MediaCenterInterface* mediaCenterInterface);

signals:
    void sigChanged(Album*);
    void artistObjChanged();
    void modelItemChanged();

private:
    mediaelch::DirectoryPath m_path;
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
    QMap<ImageType, QVector<Poster>> m_images;
    QMap<ImageType, QByteArray> m_rawImages;
    QVector<ImageType> m_imagesToRemove;
    MusicModelItem* m_modelItem;
    QString m_nfoContent;
    int m_databaseId;
    Artist* m_artistObj;
    AlbumController* m_controller;
    MusicBrainzId m_mbAlbumId;
    MusicBrainzId m_mbReleaseGroupId;
    AllMusicId m_allMusicId;
    ImageModel* m_bookletModel;
    ImageProxyModel* m_bookletProxyModel;
};
