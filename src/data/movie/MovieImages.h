#pragma once

#include "data/Poster.h"
#include "globals/Globals.h"
#include "scrapers/ScraperInfos.h"

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>

class MediaCenterInterface;
class Movie;

class MovieImages
{
public:
    explicit MovieImages(Movie& movie);
    void clear(QSet<MovieScraperInfo> infos);

    QVector<Poster> posters() const;
    QVector<Poster> backdrops() const;
    QVector<Poster> discArts() const;
    QVector<Poster> clearArts() const;
    QVector<Poster> logos() const;
    QVector<ExtraFanart> extraFanarts(MediaCenterInterface* mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QVector<QByteArray> extraFanartToAdd();
    QSet<ImageType> imagesToRemove() const;

    void addPoster(Poster poster, bool primaryLang = false);
    void addBackdrop(Poster backdrop);
    void addDiscArt(Poster discArt);
    void addClearArt(Poster clearArt);
    void addLogo(Poster logo);
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
    QList<Poster> m_posters;
    QList<Poster> m_backdrops;
    QList<Poster> m_discArts;
    QList<Poster> m_clearArts;
    QList<Poster> m_logos;

    QStringList m_extraFanartsToRemove;
    QStringList m_extraFanarts;
    int m_numPrimaryLangPosters{0};
    bool m_hasExtraFanarts{false};

    QMap<ImageType, QByteArray> m_images;
    QMap<ImageType, bool> m_hasImage;
    QMap<ImageType, bool> m_hasImageChanged;
    QList<QByteArray> m_extraFanartToAdd;
    QSet<ImageType> m_imagesToRemove;

    Movie& m_movie;
};
