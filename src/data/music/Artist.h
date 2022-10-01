#pragma once

#include "data/AllMusicId.h"
#include "data/MusicBrainzId.h"
#include "data/Poster.h"
#include "data/music/ArtistController.h"
#include "database/DatabaseId.h"
#include "globals/Globals.h"
#include "media/Path.h"
#include "model/music/MusicModelItem.h"
#include "scrapers/ScraperInfos.h"

#include <QObject>

class ArtistController;
class MusicModelItem;

class Artist : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MusicModelItem* modelItem READ modelItem NOTIFY modelItemChanged)
public:
    explicit Artist(mediaelch::DirectoryPath path = mediaelch::DirectoryPath(), QObject* parent = nullptr);
    ~Artist() override = default;

    struct Exporter;
    /// \brief Write all fields to the given exporter.
    /// \see Artist::Exporter
    void exportTo(Exporter& exporter) const;

    const mediaelch::DirectoryPath& path() const;
    void setPath(const mediaelch::DirectoryPath& path);

    QString name() const;
    void setName(const QString& name);

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

    QString yearsActive() const;
    void setYearsActive(const QString& yearsActive);

    QString formed() const;
    void setFormed(const QString& formed);

    QString born() const;
    void setBorn(const QString& born);

    QString died() const;
    void setDied(const QString& died);

    QString disbanded() const;
    void setDisbanded(const QString& disbanded);

    QString biography() const;
    void setBiography(const QString& biography);

    QVector<Poster> images(ImageType imageType) const;
    void addImage(ImageType imageType, Poster image);

    QByteArray rawImage(ImageType imageType);
    void setRawImage(ImageType imageType, QByteArray image);
    void removeImage(ImageType imageType);
    void clearImages();

    bool hasChanged() const;
    void setHasChanged(bool hasChanged);

    void clear();
    void clear(QSet<MusicScraperInfo> infos);

    MusicModelItem* modelItem() const;
    void setModelItem(MusicModelItem* modelItem);

    QString nfoContent() const;
    void setNfoContent(const QString& nfoContent);

    static QSet<ImageType> imageTypes();

    QSet<ImageType> imagesToRemove() const;
    void setImagesToRemove(const QSet<ImageType>& imagesToRemove);

    mediaelch::DatabaseId databaseId() const;
    void setDatabaseId(mediaelch::DatabaseId databaseId);

    ArtistController* controller() const;
    void setController(ArtistController* controller);

    MusicBrainzId mbId() const;
    void setMbId(const MusicBrainzId& mbId);

    QVector<Album*> albums() const;
    void setAlbums(const QVector<Album*>& albums);
    void addAlbum(Album* album);

    AllMusicId allMusicId() const;
    void setAllMusicId(const AllMusicId& allMusicId);

    QVector<ExtraFanart> extraFanarts(MediaCenterInterface* mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QVector<QByteArray> extraFanartImagesToAdd();
    void addExtraFanart(QByteArray fanart);
    void removeExtraFanart(QByteArray fanart);
    void removeExtraFanart(QString file);
    void clearExtraFanartData();

    void setDiscographyAlbums(QVector<DiscographyAlbum> albums);
    void addDiscographyAlbum(DiscographyAlbum album);
    void removeDiscographyAlbum(DiscographyAlbum* album);
    QVector<DiscographyAlbum> discographyAlbums() const;
    QVector<DiscographyAlbum*> discographyAlbumsPointer();

public:
    /// \brief   Export interface for {\see Artist::exportTo()}.
    /// \details Implement this interface and pass instances of it to {\see Artist::exportTo()}
    ///          to export the artist's data.  By using this Exporter, you ensure that
    ///          you will get notified of new fields (due to compilation errors).
    /// \todo    This structure does not export _everything_, for example m_nfoContent,
    ///          since that should not be part of an artist's data, but is more of an
    ///          implementation detail.  The Artist class needs some refactoring.
    struct Exporter
    {
        virtual void startExport() = 0;
        virtual void endExport() = 0;

        virtual void exportDatabaseId(const mediaelch::DatabaseId& databaseId) = 0;
        virtual void exporTmbId(const MusicBrainzId& mbId) = 0;
        virtual void exportAllMusicId(const AllMusicId& allMusicId) = 0;

        virtual void exportName(const QString& name) = 0;
        virtual void exportBiography(const QString& biography) = 0;
        virtual void exportDiscography(const QVector<DiscographyAlbum>& discography) = 0;
        virtual void exportGenres(const QStringList& genres) = 0;
        virtual void exportStyles(const QStringList& styles) = 0;
        virtual void exportMoods(const QStringList& moods) = 0;
        virtual void exportYearsActive(const QString& yearsActive) = 0;
        virtual void exportFormed(const QString& formed) = 0;
        virtual void exportBorn(const QString& born) = 0;
        virtual void exportDied(const QString& died) = 0;
        virtual void exportDisbanded(const QString& disbanded) = 0;
        virtual void exportImages(const QMap<ImageType, QVector<Poster>>& images) = 0;

        virtual void exportExtraFanarts(const QStringList& extraFanarts) = 0;
        virtual void exportPath(const mediaelch::DirectoryPath& path) = 0;
    };

signals:
    void sigChanged(Artist*);
    void modelItemChanged();

private:
    mediaelch::DirectoryPath m_path;
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
    QMap<ImageType, QVector<Poster>> m_images;
    QMap<ImageType, QByteArray> m_rawImages;
    QSet<ImageType> m_imagesToRemove;
    MusicModelItem* m_modelItem;
    QString m_nfoContent;
    mediaelch::DatabaseId m_databaseId;
    ArtistController* m_controller;
    MusicBrainzId m_mbId;
    AllMusicId m_allMusicId;
    QVector<Album*> m_albums;
    QVector<DiscographyAlbum> m_discography;

    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    QVector<QByteArray> m_extraFanartImagesToAdd;
};
