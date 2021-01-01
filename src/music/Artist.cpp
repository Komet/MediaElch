#include "music/Artist.h"

#include "media_centers/MediaCenterInterface.h"

#include <utility>

Artist::Artist(mediaelch::DirectoryPath path, QObject* parent) :
    QObject(parent),
    m_path{std::move(path)},
    m_hasChanged{false},
    m_modelItem{nullptr},
    m_databaseId{-1},
    m_controller{new ArtistController(this)}
{
}

const mediaelch::DirectoryPath& Artist::path() const
{
    return m_path;
}

void Artist::setPath(const mediaelch::DirectoryPath& path)
{
    m_path = path;
    setHasChanged(true);
}

QString Artist::name() const
{
    return m_name;
}

void Artist::setName(const QString& name)
{
    m_name = name;
    setHasChanged(true);
}

QStringList Artist::genres() const
{
    return m_genres;
}

void Artist::setGenres(const QStringList& genres)
{
    m_genres = genres;
    setHasChanged(true);
}

void Artist::addGenre(const QString& genre)
{
    if (genre.isEmpty()) {
        return;
    }

    m_genres.append(genre);
    setHasChanged(true);
}

void Artist::removeGenre(const QString& genre)
{
    m_genres.removeAll(genre);
    setHasChanged(true);
}

QStringList Artist::styles() const
{
    return m_styles;
}

void Artist::setStyles(const QStringList& styles)
{
    m_styles = styles;
    setHasChanged(true);
}

void Artist::addStyle(const QString& style)
{
    if (style.isEmpty()) {
        return;
    }

    m_styles.append(style);
    setHasChanged(true);
}

void Artist::removeStyle(const QString& style)
{
    m_styles.removeAll(style);
    setHasChanged(true);
}

QStringList Artist::moods() const
{
    return m_moods;
}

void Artist::setMoods(const QStringList& moods)
{
    m_moods = moods;
    setHasChanged(true);
}

void Artist::addMood(const QString& mood)
{
    if (mood.isEmpty()) {
        return;
    }

    m_moods.append(mood);
    setHasChanged(true);
}

void Artist::removeMood(const QString& mood)
{
    m_moods.removeAll(mood);
}

QString Artist::yearsActive() const
{
    return m_yearsActive;
}

void Artist::setYearsActive(const QString& yearsActive)
{
    m_yearsActive = yearsActive;
    setHasChanged(true);
}

QString Artist::formed() const
{
    return m_formed;
}

void Artist::setFormed(const QString& formed)
{
    m_formed = formed;
    setHasChanged(true);
}

QString Artist::born() const
{
    return m_born;
}

void Artist::setBorn(const QString& born)
{
    m_born = born;
    setHasChanged(true);
}

QString Artist::died() const
{
    return m_died;
}

void Artist::setDied(const QString& died)
{
    m_died = died;
    setHasChanged(true);
}

QString Artist::disbanded() const
{
    return m_disbanded;
}

void Artist::setDisbanded(const QString& disbanded)
{
    m_disbanded = disbanded;
    setHasChanged(true);
}

QString Artist::biography() const
{
    return m_biography;
}

void Artist::setBiography(const QString& biography)
{
    m_biography = biography;
    setHasChanged(true);
}

QVector<Poster> Artist::images(ImageType imageType) const
{
    return m_images.value(imageType);
}

void Artist::addImage(ImageType imageType, Poster image)
{
    if (!m_images.contains(imageType)) {
        m_images.insert(imageType, QVector<Poster>());
    }
    m_images[imageType].append(image);
    setHasChanged(true);
}

QByteArray Artist::rawImage(ImageType imageType)
{
    return m_rawImages.value(imageType);
}

void Artist::setRawImage(ImageType imageType, QByteArray image)
{
    m_rawImages.insert(imageType, image);
    setHasChanged(true);
}

void Artist::removeImage(ImageType imageType)
{
    if (!m_rawImages.value(imageType, QByteArray()).isNull()) {
        m_rawImages.remove(imageType);
    } else if (!m_imagesToRemove.contains(imageType)) {
        m_imagesToRemove.append(imageType);
    }
    setHasChanged(true);
}

void Artist::clearImages()
{
    m_rawImages.clear();
    m_imagesToRemove.clear();
    m_extraFanartImagesToAdd.clear();
    m_extraFanartsToRemove.clear();
}

bool Artist::hasChanged() const
{
    return m_hasChanged;
}

void Artist::setHasChanged(bool hasChanged)
{
    m_hasChanged = hasChanged;
    if (hasChanged) {
        emit sigChanged(this);
    }
}

void Artist::clear()
{
    QSet<MusicScraperInfo> infos;
    infos << MusicScraperInfo::Name        //
          << MusicScraperInfo::Genres      //
          << MusicScraperInfo::Styles      //
          << MusicScraperInfo::Moods       //
          << MusicScraperInfo::YearsActive //
          << MusicScraperInfo::Formed      //
          << MusicScraperInfo::Died        //
          << MusicScraperInfo::Born        //
          << MusicScraperInfo::Disbanded   //
          << MusicScraperInfo::Biography   //
          << MusicScraperInfo::Thumb       //
          << MusicScraperInfo::Fanart      //
          << MusicScraperInfo::Logo        //
          << MusicScraperInfo::Discography //
          << MusicScraperInfo::ExtraFanarts;
    clear(infos);
    m_nfoContent.clear();
}

void Artist::clear(QSet<MusicScraperInfo> infos)
{
    if (infos.contains(MusicScraperInfo::Genres)) {
        m_genres.clear();
    }
    if (infos.contains(MusicScraperInfo::Styles)) {
        m_styles.clear();
    }
    if (infos.contains(MusicScraperInfo::Moods)) {
        m_moods.clear();
    }
    if (infos.contains(MusicScraperInfo::YearsActive)) {
        m_yearsActive.clear();
    }
    if (infos.contains(MusicScraperInfo::Formed)) {
        m_formed.clear();
    }
    if (infos.contains(MusicScraperInfo::Born)) {
        m_born.clear();
    }
    if (infos.contains(MusicScraperInfo::Died)) {
        m_died.clear();
    }
    if (infos.contains(MusicScraperInfo::Disbanded)) {
        m_disbanded.clear();
    }
    if (infos.contains(MusicScraperInfo::Biography)) {
        m_biography.clear();
    }

    if (infos.contains(MusicScraperInfo::Thumb)) {
        if (!m_images.contains(ImageType::ArtistThumb)) {
            m_images.insert(ImageType::ArtistThumb, QVector<Poster>());
        }
        m_images[ImageType::ArtistThumb].clear();
        m_rawImages.insert(ImageType::ArtistThumb, QByteArray());
    }
    if (infos.contains(MusicScraperInfo::Fanart)) {
        if (!m_images.contains(ImageType::ArtistFanart)) {
            m_images.insert(ImageType::ArtistFanart, QVector<Poster>());
        }
        m_images[ImageType::ArtistFanart].clear();
        m_rawImages.insert(ImageType::ArtistFanart, QByteArray());
    }
    if (infos.contains(MusicScraperInfo::Logo)) {
        if (!m_images.contains(ImageType::ArtistLogo)) {
            m_images.insert(ImageType::ArtistLogo, QVector<Poster>());
        }
        m_images[ImageType::ArtistLogo].clear();
        m_rawImages.insert(ImageType::ArtistLogo, QByteArray());
    }
    if (infos.contains(MusicScraperInfo::ExtraFanarts)) {
        m_extraFanartsToRemove.clear();
        m_extraFanartImagesToAdd.clear();
        m_extraFanarts.clear();
    }
    if (infos.contains(MusicScraperInfo::Discography)) {
        m_discography.clear();
    }
}

MusicModelItem* Artist::modelItem() const
{
    return m_modelItem;
}

void Artist::setModelItem(MusicModelItem* modelItem)
{
    m_modelItem = modelItem;
    emit modelItemChanged();
}

QString Artist::nfoContent() const
{
    return m_nfoContent;
}

void Artist::setNfoContent(const QString& nfoContent)
{
    m_nfoContent = nfoContent;
}

QVector<ImageType> Artist::imageTypes()
{
    return {ImageType::ArtistThumb, ImageType::ArtistLogo, ImageType::ArtistFanart};
}

QVector<ImageType> Artist::imagesToRemove() const
{
    return m_imagesToRemove;
}

void Artist::setImagesToRemove(const QVector<ImageType>& imagesToRemove)
{
    m_imagesToRemove = imagesToRemove;
}

int Artist::databaseId() const
{
    return m_databaseId;
}

void Artist::setDatabaseId(int databaseId)
{
    m_databaseId = databaseId;
}

ArtistController* Artist::controller() const
{
    return m_controller;
}

void Artist::setController(ArtistController* controller)
{
    m_controller = controller;
}

MusicBrainzId Artist::mbId() const
{
    return m_mbId;
}

void Artist::setMbId(const MusicBrainzId& mbId)
{
    m_mbId = mbId;
}

QVector<Album*> Artist::albums() const
{
    return m_albums;
}

void Artist::setAlbums(const QVector<Album*>& albums)
{
    m_albums = albums;
}

void Artist::addAlbum(Album* album)
{
    m_albums.append(album);
}

AllMusicId Artist::allMusicId() const
{
    return m_allMusicId;
}

void Artist::setAllMusicId(const AllMusicId& allMusicId)
{
    m_allMusicId = allMusicId;
}

void Artist::addExtraFanart(QByteArray fanart)
{
    m_extraFanartImagesToAdd.append(fanart);
    setHasChanged(true);
}

void Artist::removeExtraFanart(QByteArray fanart)
{
    m_extraFanartImagesToAdd.removeOne(fanart);
    setHasChanged(true);
}

void Artist::removeExtraFanart(QString file)
{
    m_extraFanarts.removeOne(file);
    m_extraFanartsToRemove.append(file);
    setHasChanged(true);
}

QVector<ExtraFanart> Artist::extraFanarts(MediaCenterInterface* mediaCenterInterface)
{
    if (m_extraFanarts.isEmpty()) {
        m_extraFanarts = mediaCenterInterface->extraFanartNames(this);
    }
    for (const QString& file : m_extraFanartsToRemove) {
        m_extraFanarts.removeOne(file);
    }
    QVector<ExtraFanart> fanarts;
    for (const QString& file : m_extraFanarts) {
        ExtraFanart f;
        f.path = file;
        fanarts.append(f);
    }
    for (const QByteArray& img : m_extraFanartImagesToAdd) {
        ExtraFanart f;
        f.image = img;
        fanarts.append(f);
    }
    return fanarts;
}

QStringList Artist::extraFanartsToRemove()
{
    return m_extraFanartsToRemove;
}

QVector<QByteArray> Artist::extraFanartImagesToAdd()
{
    return m_extraFanartImagesToAdd;
}

void Artist::clearExtraFanartData()
{
    m_extraFanartImagesToAdd.clear();
    m_extraFanartsToRemove.clear();
    m_extraFanarts.clear();
}

void Artist::setDiscographyAlbums(QVector<DiscographyAlbum> albums)
{
    m_discography = albums;
    setHasChanged(true);
}

void Artist::addDiscographyAlbum(DiscographyAlbum album)
{
    m_discography.append(album);
    setHasChanged(true);
}

void Artist::removeDiscographyAlbum(DiscographyAlbum* album)
{
    for (int i = 0, n = m_discography.size(); i < n; ++i) {
        if (&m_discography[i] == album) {
            m_discography.removeAt(i);
            break;
        }
    }
    setHasChanged(true);
}

QVector<DiscographyAlbum> Artist::discographyAlbums() const
{
    return m_discography;
}

QVector<DiscographyAlbum*> Artist::discographyAlbumsPointer()
{
    QVector<DiscographyAlbum*> albums;
    for (auto& i : m_discography) {
        albums.append(&i);
    }
    return albums;
}
