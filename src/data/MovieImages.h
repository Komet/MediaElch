#ifndef DATA_MOVIEIMAGES_H
#define DATA_MOVIEIMAGES_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>

#include "globals/Globals.h"

class MediaCenterInterface;
class Movie;

class MovieImages
{
public:
    explicit MovieImages(Movie &movie);
    void clear(QList<MovieScraperInfos> infos);

    QList<Poster> posters() const;
    QList<Poster> backdrops() const;
    QList<Poster> discArts() const;
    QList<Poster> clearArts() const;
    QList<Poster> logos() const;
    QList<ExtraFanart> extraFanarts(MediaCenterInterface *mediaCenterInterface);
    QStringList extraFanartsToRemove();
    QList<QByteArray> extraFanartToAdd();
    QList<ImageType> imagesToRemove() const;

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
    QList<ImageType> m_imagesToRemove;

    Movie &m_movie;
};

#endif // DATA_MOVIEIMAGES_H
