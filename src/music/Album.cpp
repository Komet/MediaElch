#include "music/Album.h"

#include <utility>

#include "media_centers/MediaCenterInterface.h"

Album::Album(mediaelch::DirectoryPath path, QObject* parent) :
    QObject(parent),
    m_path{std::move(path)},
    m_hasChanged{false},
    m_rating{0},
    m_year{0},
    m_modelItem{nullptr},
    m_databaseId{-1},
    m_artistObj{nullptr},
    m_controller{new AlbumController(this)},
    m_bookletModel{new ImageModel(this)},
    m_bookletProxyModel{new ImageProxyModel(this)}
{
    m_bookletProxyModel->setSourceModel(m_bookletModel);
}

const mediaelch::DirectoryPath& Album::path() const
{
    return m_path;
}

void Album::setPath(const mediaelch::DirectoryPath& path)
{
    m_path = path;
    setHasChanged(true);
}

bool Album::hasChanged() const
{
    return m_hasChanged;
}

void Album::setHasChanged(bool hasChanged)
{
    m_hasChanged = hasChanged;
    if (hasChanged) {
        emit sigChanged(this);
    }
}

QString Album::title() const
{
    return m_title;
}

void Album::setTitle(const QString& title)
{
    m_title = title;
    setHasChanged(true);
}

QString Album::artist() const
{
    return m_artist;
}

void Album::setArtist(const QString& artist)
{
    m_artist = artist;
    setHasChanged(true);
}

QStringList Album::genres() const
{
    return m_genres;
}

void Album::setGenres(const QStringList& genres)
{
    m_genres = genres;
    setHasChanged(true);
}

void Album::addGenre(const QString& genre)
{
    if (genre.isEmpty()) {
        return;
    }
    m_genres.append(genre);
    setHasChanged(true);
}

void Album::removeGenre(const QString& genre)
{
    m_genres.removeAll(genre);
    setHasChanged(true);
}

QStringList Album::styles() const
{
    return m_styles;
}

void Album::setStyles(const QStringList& styles)
{
    m_styles = styles;
    setHasChanged(true);
}

void Album::addStyle(const QString& style)
{
    if (style.isEmpty()) {
        return;
    }
    m_styles.append(style);
    setHasChanged(true);
}

void Album::removeStyle(const QString& style)
{
    m_styles.removeAll(style);
    setHasChanged(true);
}

QStringList Album::moods() const
{
    return m_moods;
}

void Album::setMoods(const QStringList& moods)
{
    m_moods = moods;
    setHasChanged(true);
}

void Album::addMood(const QString& mood)
{
    if (mood.isEmpty()) {
        return;
    }
    m_moods.append(mood);
    setHasChanged(true);
}

void Album::removeMood(const QString& mood)
{
    m_moods.removeAll(mood);
    setHasChanged(true);
}

QString Album::review() const
{
    return m_review;
}

void Album::setReview(const QString& review)
{
    m_review = review;
    setHasChanged(true);
}

QString Album::releaseDate() const
{
    return m_releaseDate;
}

void Album::setReleaseDate(const QString& releaseDate)
{
    m_releaseDate = releaseDate;
    setHasChanged(true);
}

QString Album::label() const
{
    return m_label;
}

void Album::setLabel(const QString& label)
{
    m_label = label;
    setHasChanged(true);
}

qreal Album::rating() const
{
    return m_rating;
}

void Album::setRating(const qreal& rating)
{
    m_rating = rating;
    setHasChanged(true);
}

int Album::year() const
{
    return m_year;
}

void Album::setYear(int year)
{
    m_year = year;
    setHasChanged(true);
}

QVector<Poster> Album::images(ImageType imageType) const
{
    return m_images.value(imageType);
}

void Album::addImage(ImageType imageType, Poster image)
{
    if (!m_images.contains(imageType)) {
        m_images.insert(imageType, QVector<Poster>());
    }
    m_images[imageType].append(image);
    setHasChanged(true);
}

QByteArray Album::rawImage(ImageType imageType)
{
    return m_rawImages.value(imageType);
}

void Album::setRawImage(ImageType imageType, QByteArray image)
{
    m_rawImages.insert(imageType, image);
    setHasChanged(true);
}

void Album::removeImage(ImageType imageType)
{
    if (!m_rawImages.value(imageType, QByteArray()).isNull()) {
        m_rawImages.remove(imageType);
    } else if (!m_imagesToRemove.contains(imageType)) {
        m_imagesToRemove.append(imageType);
    }
    setHasChanged(true);
}

void Album::clearImages()
{
    m_rawImages.clear();
    m_imagesToRemove.clear();
}

void Album::clear()
{
    QSet<MusicScraperInfo> infos;
    infos << MusicScraperInfo::Title       //
          << MusicScraperInfo::Artist      //
          << MusicScraperInfo::Genres      //
          << MusicScraperInfo::Styles      //
          << MusicScraperInfo::Moods       //
          << MusicScraperInfo::Review      //
          << MusicScraperInfo::ReleaseDate //
          << MusicScraperInfo::Label       //
          << MusicScraperInfo::Rating      //
          << MusicScraperInfo::Year        //
          << MusicScraperInfo::Cover       //
          << MusicScraperInfo::CdArt;
    clear(infos);
    m_nfoContent.clear();
}

void Album::clear(QSet<MusicScraperInfo> infos)
{
    if (infos.contains(MusicScraperInfo::Artist)) {
        m_artist.clear();
    }
    if (infos.contains(MusicScraperInfo::Title)) {
        m_title.clear();
    }
    if (infos.contains(MusicScraperInfo::Genres)) {
        m_genres.clear();
    }
    if (infos.contains(MusicScraperInfo::Styles)) {
        m_styles.clear();
    }
    if (infos.contains(MusicScraperInfo::Moods)) {
        m_moods.clear();
    }
    if (infos.contains(MusicScraperInfo::Review)) {
        m_review.clear();
    }
    if (infos.contains(MusicScraperInfo::ReleaseDate)) {
        m_releaseDate.clear();
    }
    if (infos.contains(MusicScraperInfo::Label)) {
        m_label.clear();
    }
    if (infos.contains(MusicScraperInfo::Rating)) {
        m_rating = 0;
    }
    if (infos.contains(MusicScraperInfo::Year)) {
        m_year = 0;
    }

    if (infos.contains(MusicScraperInfo::Cover)) {
        if (!m_images.contains(ImageType::AlbumThumb)) {
            m_images.insert(ImageType::AlbumThumb, QVector<Poster>());
        }
        m_images[ImageType::AlbumThumb].clear();
        m_rawImages.insert(ImageType::AlbumThumb, QByteArray());
    }
    if (infos.contains(MusicScraperInfo::CdArt)) {
        if (!m_images.contains(ImageType::AlbumCdArt)) {
            m_images.insert(ImageType::AlbumCdArt, QVector<Poster>());
        }
        m_images[ImageType::AlbumCdArt].clear();
        m_rawImages.insert(ImageType::AlbumCdArt, QByteArray());
    }
}

MusicModelItem* Album::modelItem() const
{
    return m_modelItem;
}

void Album::setModelItem(MusicModelItem* item)
{
    m_modelItem = item;
    emit modelItemChanged();
}

QString Album::nfoContent() const
{
    return m_nfoContent;
}

void Album::setNfoContent(const QString& nfoContent)
{
    m_nfoContent = nfoContent;
}

QVector<ImageType> Album::imageTypes()
{
    return {ImageType::AlbumThumb, ImageType::AlbumCdArt};
}

QVector<ImageType> Album::imagesToRemove() const
{
    return m_imagesToRemove;
}

void Album::setImagesToRemove(const QVector<ImageType>& imagesToRemove)
{
    m_imagesToRemove = imagesToRemove;
}

int Album::databaseId() const
{
    return m_databaseId;
}

void Album::setDatabaseId(int databaseId)
{
    m_databaseId = databaseId;
}

Artist* Album::artistObj() const
{
    return m_artistObj;
}

void Album::setArtistObj(Artist* artistObj)
{
    m_artistObj = artistObj;
    emit artistObjChanged();
}

AlbumController* Album::controller() const
{
    return m_controller;
}

void Album::setController(AlbumController* controller)
{
    m_controller = controller;
}

MusicBrainzId Album::mbReleaseGroupId() const
{
    return m_mbReleaseGroupId;
}

void Album::setMbReleaseGroupId(const MusicBrainzId& mbId)
{
    m_mbReleaseGroupId = mbId;
}


MusicBrainzId Album::mbAlbumId() const
{
    return m_mbAlbumId;
}

void Album::setMbAlbumId(const MusicBrainzId& mbAlbumId)
{
    m_mbAlbumId = mbAlbumId;
}

AllMusicId Album::allMusicId() const
{
    return m_allMusicId;
}

void Album::setAllMusicId(const AllMusicId& allMusicId)
{
    m_allMusicId = allMusicId;
}

ImageModel* Album::bookletModel() const
{
    return m_bookletModel;
}

ImageProxyModel* Album::bookletProxyModel() const
{
    return m_bookletProxyModel;
}

void Album::loadBooklets(MediaCenterInterface* mediaCenterInterface)
{
    m_bookletProxyModel->blockSignals(true);
    mediaCenterInterface->loadBooklets(this);
    m_bookletProxyModel->blockSignals(false);
}
