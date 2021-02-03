#include "MovieImages.h"

#include "globals/Globals.h"
#include "media_centers/MediaCenterInterface.h"
#include "movies/Movie.h"

#include <QVector>

MovieImages::MovieImages(Movie& movie) : m_movie(movie)
{
}

void MovieImages::clear(QSet<MovieScraperInfo> infos)
{
    if (infos.contains(MovieScraperInfo::Backdrop)) {
        m_backdrops.clear();
        m_images.insert(ImageType::MovieBackdrop, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieBackdrop, false);
        m_imagesToRemove.removeOne(ImageType::MovieBackdrop);
    }
    if (infos.contains(MovieScraperInfo::CdArt)) {
        m_discArts.clear();
        m_images.insert(ImageType::MovieCdArt, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieCdArt, false);
        m_imagesToRemove.removeOne(ImageType::MovieCdArt);
    }
    if (infos.contains(MovieScraperInfo::ClearArt)) {
        m_clearArts.clear();
        m_images.insert(ImageType::MovieClearArt, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieClearArt, false);
        m_imagesToRemove.removeOne(ImageType::MovieClearArt);
    }
    if (infos.contains(MovieScraperInfo::Logo)) {
        m_logos.clear();
        m_images.insert(ImageType::MovieLogo, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieLogo, false);
        m_imagesToRemove.removeOne(ImageType::MovieLogo);
    }
    if (infos.contains(MovieScraperInfo::Poster)) {
        m_posters.clear();
        m_images.insert(ImageType::MoviePoster, QByteArray());
        m_hasImageChanged.insert(ImageType::MoviePoster, false);
        m_numPrimaryLangPosters = 0;
        m_imagesToRemove.removeOne(ImageType::MoviePoster);
    }

    if (infos.contains(MovieScraperInfo::Banner)) {
        m_images.insert(ImageType::MovieBanner, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieBanner, false);
        m_imagesToRemove.removeOne(ImageType::MovieBanner);
    }
    if (infos.contains(MovieScraperInfo::Thumb)) {
        m_images.insert(ImageType::MovieThumb, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieThumb, false);
        m_imagesToRemove.removeOne(ImageType::MovieThumb);
    }
    if (infos.contains(MovieScraperInfo::ExtraFanarts)) {
        m_extraFanartsToRemove.clear();
        m_extraFanartToAdd.clear();
        m_extraFanarts.clear();
    }
}

/**
 * \property MovieImages::posters
 * \brief Holds a list of posters of the movie
 * \return List of posters
 */
QVector<Poster> MovieImages::posters() const
{
    return m_posters.toVector();
}

/**
 * \property MovieImages::backdrops
 * \brief Holds a list of backdrops of the movie
 * \return List of backdrops
 */
QVector<Poster> MovieImages::backdrops() const
{
    return m_backdrops.toVector();
}

QVector<Poster> MovieImages::discArts() const
{
    return m_discArts.toVector();
}

QVector<Poster> MovieImages::clearArts() const
{
    return m_clearArts.toVector();
}

QVector<Poster> MovieImages::logos() const
{
    return m_logos.toVector();
}

QStringList MovieImages::extraFanartsToRemove()
{
    return m_extraFanartsToRemove;
}

QVector<QByteArray> MovieImages::extraFanartToAdd()
{
    return m_extraFanartToAdd.toVector();
}

QVector<ImageType> MovieImages::imagesToRemove() const
{
    return m_imagesToRemove.toVector();
}

/**
 * \brief Returns how many of the posters were scraped in the primary language
 * \return Number of primary language posters
 */
int MovieImages::numPrimaryLangPosters() const
{
    return m_numPrimaryLangPosters;
}

/**
 * \brief Sets the number of primary language posters
 * \param numberPrimaryLangPosters Number of primary language posters
 */
void MovieImages::setNumPrimaryLangPosters(int numberPrimaryLangPosters)
{
    m_numPrimaryLangPosters = numberPrimaryLangPosters;
}

void MovieImages::addPoster(Poster poster, bool primaryLang)
{
    if (primaryLang) {
        m_posters.insert(m_numPrimaryLangPosters, poster);
        m_numPrimaryLangPosters++;
    } else {
        m_posters.append(poster);
    }
    m_movie.setChanged(true);
}

void MovieImages::addBackdrop(Poster backdrop)
{
    m_backdrops.append(backdrop);
    m_movie.setChanged(true);
}

void MovieImages::addDiscArt(Poster discArt)
{
    m_discArts.append(discArt);
    m_movie.setChanged(true);
}

void MovieImages::addClearArt(Poster clearArt)
{
    m_clearArts.append(clearArt);
    m_movie.setChanged(true);
}

void MovieImages::addLogo(Poster logo)
{
    m_logos.append(logo);
    m_movie.setChanged(true);
}


void MovieImages::addExtraFanart(QByteArray fanart)
{
    m_extraFanartToAdd.append(fanart);
    m_movie.setChanged(true);
}

void MovieImages::removeExtraFanart(QByteArray fanart)
{
    m_extraFanartToAdd.removeOne(fanart);
    m_movie.setChanged(true);
}

void MovieImages::removeExtraFanart(QString file)
{
    m_extraFanarts.removeOne(file);
    m_extraFanartsToRemove.append(file);
    m_movie.setChanged(true);
}

QVector<ExtraFanart> MovieImages::extraFanarts(MediaCenterInterface* mediaCenterInterface)
{
    if (m_extraFanarts.isEmpty()) {
        m_extraFanarts = mediaCenterInterface->extraFanartNames(&m_movie);
    }
    for (const QString& file : asConst(m_extraFanartsToRemove)) {
        m_extraFanarts.removeOne(file);
    }
    QVector<ExtraFanart> fanarts;
    for (const QString& file : asConst(m_extraFanarts)) {
        ExtraFanart f;
        f.path = file;
        fanarts.append(f);
    }
    for (const QByteArray& img : asConst(m_extraFanartToAdd)) {
        ExtraFanart f;
        f.image = img;
        fanarts.append(f);
    }
    return fanarts;
}

void MovieImages::clearExtraFanartData()
{
    m_extraFanartToAdd.clear();
    m_extraFanartsToRemove.clear();
    m_extraFanarts.clear();
}

void MovieImages::removeImage(ImageType type)
{
    if (!m_images.value(type, QByteArray()).isNull()) {
        m_images.remove(type);
        m_hasImageChanged.insert(type, false);
    } else if (!m_imagesToRemove.contains(type)) {
        m_imagesToRemove.append(type);
    }
    m_movie.setChanged(true);
}

QByteArray MovieImages::image(ImageType imageType) const
{
    return m_images.value(imageType, QByteArray());
}

bool MovieImages::imageHasChanged(ImageType imageType)
{
    return m_hasImageChanged.value(imageType, false);
}

bool MovieImages::hasImage(ImageType imageType) const
{
    return m_hasImage.value(imageType, false);
}

void MovieImages::setHasImage(ImageType imageType, bool has)
{
    m_hasImage.insert(imageType, has);
}

void MovieImages::setImage(ImageType imageType, QByteArray image)
{
    m_images.insert(imageType, image);
    m_hasImageChanged.insert(imageType, true);
    m_movie.setChanged(true);
}

void MovieImages::setHasExtraFanarts(bool has)
{
    m_hasExtraFanarts = has;
}

bool MovieImages::hasExtraFanarts() const
{
    return m_hasExtraFanarts;
}

/**
 * \brief Clears the movie images to save memory
 */
void MovieImages::clearImages()
{
    m_images.clear();
    m_hasImageChanged.clear();
    m_extraFanartToAdd.clear();
}
