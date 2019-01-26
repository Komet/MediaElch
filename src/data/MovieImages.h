#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVector>

#include "globals/Globals.h"

class MediaCenterInterface;
class Movie;

class MovieImages
{
public:
    explicit MovieImages(Movie &movie);
    void clear(QVector<MovieScraperInfos> infos);

    QVector<Poster> posters() const;
    QVector<Poster> backdrops() const;
    QVector<Poster> discArts() const;
    QVector<Poster> clearArts() const;
    QVector<Poster> logos() const;
    QVector<ExtraFanart> extraFanarts(MediaCenterInterface *mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QVector<QByteArray> extraFanartToAdd();
    QVector<ImageType> imagesToRemove() const;

    void addPoster(Poster poster, bool primaryLang = false);
    void addBackdrop(Poster backdrop);
    void addDiscArt(Poster poster);
    void addClearArt(Poster poster);
    void addLogo(Poster poster);
    void addExtraFanart(QByteArray fanart);
    void removeExtraFanart(QByteArray fanart);
    void removeExtraFanart(QString file);
    void clearExtraFanartData();
    void clearImages();
    void removeImage(ImageType type);

    int numPrimaryLangPosters() const;

    void setNumPrimaryLangPosters(int numberPrimaryLangPosters);

    // Images
    bool hasExtraFanarts() const;
    void setHasExtraFanarts(bool has);
    QByteArray image(ImageType imageType) const;
    bool imageHasChanged(ImageType imageType);
    void setHasImage(ImageType imageType, bool has);
    bool hasImage(ImageType imageType) const;
    void setImage(ImageType imageType, QByteArray image);

private:
    QVector<Poster> m_posters;
    QVector<Poster> m_backdrops;
    QVector<Poster> m_discArts;
    QVector<Poster> m_clearArts;
    QVector<Poster> m_logos;

    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    int m_numPrimaryLangPosters{0};
    bool m_hasExtraFanarts{false};

    QMap<ImageType, QByteArray> m_images;
    QMap<ImageType, bool> m_hasImage;
    QMap<ImageType, bool> m_hasImageChanged;
    QVector<QByteArray> m_extraFanartToAdd;
    QVector<ImageType> m_imagesToRemove;

    Movie &m_movie;
};
