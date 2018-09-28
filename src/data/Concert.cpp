#include "Concert.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "data/MediaCenterInterface.h"
#include "data/StreamDetails.h"
#include "globals/Helper.h"
#include "globals/NameFormatter.h"
#include "settings/Settings.h"

using namespace std::chrono_literals;

/**
 * @brief Constructs a new concert object
 * @param files List of files for this concert
 * @param parent
 */
Concert::Concert(QStringList files, QObject *parent) :
    QObject(parent),
    m_controller{new ConcertController(this)},
    m_rating{0},
    m_runtime{0min},
    m_playcount{0},
    m_downloadsSize{0},
    m_watched{false},
    m_hasChanged{false},
    m_downloadsInProgress{false},
    m_inSeparateFolder{false},
    m_mediaCenterId{-1},
    m_streamDetailsLoaded{false},
    m_databaseId{-1},
    m_syncNeeded{false},
    m_hasExtraFanarts{false}
{
    moveToThread(QApplication::instance()->thread());
    static int m_idCounter = 0;
    m_concertId = ++m_idCounter;
    setFiles(files);
}

void Concert::setFiles(QStringList files)
{
    m_files = files;
    m_streamDetails = new StreamDetails(this, files);
    if (!files.isEmpty()) {
        QFileInfo fi(files.at(0));
        QStringList path = fi.path().split("/", QString::SkipEmptyParts);
        m_folderName = path.last();
    }
}

/**
 * @brief Clears all infos in the concert
 */
void Concert::clear()
{
    QList<ConcertScraperInfos> infos;
    infos.reserve(14);
    infos << ConcertScraperInfos::Title         //
          << ConcertScraperInfos::Tagline       //
          << ConcertScraperInfos::Rating        //
          << ConcertScraperInfos::Released      //
          << ConcertScraperInfos::Runtime       //
          << ConcertScraperInfos::Certification //
          << ConcertScraperInfos::Trailer       //
          << ConcertScraperInfos::Overview      //
          << ConcertScraperInfos::Poster        //
          << ConcertScraperInfos::Backdrop      //
          << ConcertScraperInfos::Genres        //
          << ConcertScraperInfos::Tags          //
          << ConcertScraperInfos::ExtraArts     //
          << ConcertScraperInfos::ExtraFanarts;
    clear(infos);
    m_nfoContent.clear();
}

/**
 * @brief Clears contents of the concert based on a list
 * @param infos List of infos which should be cleared
 */
void Concert::clear(QList<ConcertScraperInfos> infos)
{
    if (infos.contains(ConcertScraperInfos::Backdrop)) {
        m_backdrops.clear();
        m_images.insert(ImageType::ConcertBackdrop, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertBackdrop, false);
        m_imagesToRemove.removeOne(ImageType::ConcertBackdrop);
    }
    if (infos.contains(ConcertScraperInfos::Genres)) {
        m_genres.clear();
    }
    if (infos.contains(ConcertScraperInfos::Poster)) {
        m_posters.clear();
        m_images.insert(ImageType::ConcertPoster, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertPoster, false);
        m_imagesToRemove.removeOne(ImageType::ConcertPoster);
    }
    if (infos.contains(ConcertScraperInfos::Overview)) {
        m_overview = "";
    }
    if (infos.contains(ConcertScraperInfos::Rating)) {
        m_rating = 0;
    }
    if (infos.contains(ConcertScraperInfos::Released)) {
        m_released = QDate(2000, 02, 30); // invalid date
    }
    if (infos.contains(ConcertScraperInfos::Tagline)) {
        m_tagline = "";
    }
    if (infos.contains(ConcertScraperInfos::Runtime)) {
        m_runtime = 0min;
    }
    if (infos.contains(ConcertScraperInfos::Trailer)) {
        m_trailer = "";
    }
    if (infos.contains(ConcertScraperInfos::Certification)) {
        m_certification = "";
    }
    if (infos.contains(ConcertScraperInfos::Tags)) {
        m_tags.clear();
    }
    if (infos.contains(ConcertScraperInfos::ExtraArts)) {
        m_images.insert(ImageType::ConcertCdArt, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertCdArt, false);
        m_images.insert(ImageType::ConcertLogo, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertLogo, false);
        m_images.insert(ImageType::ConcertClearArt, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertClearArt, false);
        m_imagesToRemove.removeOne(ImageType::ConcertCdArt);
        m_imagesToRemove.removeOne(ImageType::ConcertClearArt);
        m_imagesToRemove.removeOne(ImageType::ConcertLogo);
    }
    if (infos.contains(ConcertScraperInfos::ExtraFanarts)) {
        m_extraFanartsToRemove.clear();
        m_extraFanartImagesToAdd.clear();
        m_extraFanarts.clear();
    }
}

ConcertController *Concert::controller() const
{
    return m_controller;
}

/**
 * @brief Clears the concert images to save memory
 */
void Concert::clearImages()
{
    m_images.clear();
    m_hasImageChanged.clear();
    m_extraFanartImagesToAdd.clear();
}

/*** GETTER ***/

/**
 * @property Concert::name
 * @brief Holds the concerts name
 * @return The concerts name
 * @see Concert::setName
 */
QString Concert::name() const
{
    return m_name;
}

/**
 * @property Concert::artist
 * @brief Holds the concerts artist
 * @return The concerts artist
 * @see Concert::setArtist
 */
QString Concert::artist() const
{
    return m_artist;
}

/**
 * @property Concert::album
 * @brief Holds the concerts album
 * @return The concerts album
 * @see Concert::setAlbum
 */
QString Concert::album() const
{
    return m_album;
}

/**
 * @property Concert::overview
 * @brief Holds the concerts plot
 * @return Plot of the concert
 * @see Concert::setOverview
 */
QString Concert::overview() const
{
    return m_overview;
}

/**
 * @brief Holds the concerts rating
 * @return Rating of the concert
 * @see Concert::setRating
 */
qreal Concert::rating() const
{
    return m_rating;
}

/**
 * @property Concert::released
 * @brief Holds the concerts release date
 * @return Release date of the concert
 * @see Concert::setReleased
 */
QDate Concert::released() const
{
    return m_released;
}

/**
 * @property Concert::tagline
 * @brief Holds the concerts tagline
 * @return Tagline of the concert
 * @see Concert::setTagline
 */
QString Concert::tagline() const
{
    return m_tagline;
}

/**
 * @brief Holds the concerts runtime
 * @return Runtime of the concert
 * @see Concert::setRuntime
 */
std::chrono::minutes Concert::runtime() const
{
    return m_runtime;
}

/**
 * @property Concert::certification
 * @brief Holds the concerts certification
 * @return Certification of the concert
 * @see Concert::setCertification
 */
QString Concert::certification() const
{
    return m_certification;
}

/**
 * @property Concert::genres
 * @brief Holds a list of the concert genres
 * @return List of genres of the concert
 * @see Concert::setGenres
 * @see Concert::genresPointer
 * @see Concert::addGenre
 * @see Concert::removeGenre
 */
QStringList Concert::genres() const
{
    return m_genres;
}

/**
 * @brief Returns a list of pointers to QStrings
 * @return List of pointers to the concert genres
 */
QList<QString *> Concert::genresPointer()
{
    QList<QString *> genres;
    for (int i = 0, n = m_genres.size(); i < n; ++i) {
        genres.append(&m_genres[i]);
    }
    return genres;
}

/**
 * @property Concert::trailer
 * @brief Holds the concerts trailer
 * @return Trailer of the concert
 * @see Concert::setTrailer
 */
QUrl Concert::trailer() const
{
    return m_trailer;
}

/**
 * @brief Holds the files of the concert
 * @return List of files
 */
QStringList Concert::files() const
{
    return m_files;
}

/**
 * @property Concert::playcount
 * @brief Holds the playcount
 * @return Playcount of the concert
 * @see Concert::setPlayCount
 */
int Concert::playcount() const
{
    return m_playcount;
}

/**
 * @property Concert::lastPlayed
 * @brief Holds the date when the concert was last played
 *        If the concert was never played an invalid date will be returned
 * @return Date of last playtime
 * @see Concert::setLastPlayed
 */
QDateTime Concert::lastPlayed() const
{
    return m_lastPlayed;
}

/**
 * @property Concert::posters
 * @brief Holds a list of posters of the concert
 * @return List of posters
 * @see Concert::setPosters
 * @see Concert::setPoster
 * @see Concert::addPoster
 */
QList<Poster> Concert::posters() const
{
    return m_posters;
}

/**
 * @property Concert::backdrops
 * @brief Holds a list of backdrops of the concert
 * @return List of backdrops
 * @see Concert::setBackdrops
 * @see Concert::setBackdrop
 * @see Concert::addBackdrop
 */
QList<Poster> Concert::backdrops() const
{
    return m_backdrops;
}

/**
 * @brief Returns the parent folder of the concert
 * @return Parent folder of the concert
 */
QString Concert::folderName() const
{
    return m_folderName;
}

/**
 * @property Concert::streamDetailsLoaded
 * @brief Holds if the stream details were loaded
 * @return True if the stream details were loaded
 * @see Concert::setStreamDetailsLoaded
 */
bool Concert::streamDetailsLoaded() const
{
    return m_streamDetailsLoaded;
}

/**
 * @property Concert::watched
 * @brief Holds the concerts watched status
 * @return Watched status of the concert
 * @see Concert::hasWatched
 */
bool Concert::watched() const
{
    return m_watched;
}

/**
 * @property Concert::hasChanged
 * @brief Holds a property if the concert infos were changed by a setter or a ScraperInterface
 * @return True if some of the concert infos were changed
 * @see Concert::setChanged
 */
bool Concert::hasChanged() const
{
    return m_hasChanged;
}

/**
 * @brief Holds a unique MediaElch concert id
 * @return MediaElchs id of the concert
 */
int Concert::concertId() const
{
    return m_concertId;
}

/**
 * @brief Returns true if a download is in progress
 * @return Download is in progress
 */
bool Concert::downloadsInProgress() const
{
    return m_downloadsInProgress;
}

/**
 * @brief Returns how many downloads are left for this concert
 * @return Number of downloads left
 */
int Concert::downloadsSize() const
{
    return m_downloadsSize;
}

/**
 * @brief Holds if the concert files are stored in a separate folder
 * @return Concert files are stored in a separate folder
 */
bool Concert::inSeparateFolder() const
{
    return m_inSeparateFolder;
}

/**
 * @brief Concert::mediaCenterId
 * @return Id in a MediaCenterInterface
 */
int Concert::mediaCenterId() const
{
    return m_mediaCenterId;
}

/**
 * @property Concert::tmdbId
 * @brief Holds the concerts tmdb id
 * @return The concerts tmdb id
 * @see Concert::setTmdbId
 */
QString Concert::tmdbId() const
{
    return m_tmdbId;
}

/**
 * @property Concert::id
 * @brief Holds the concerts id
 * @return The concerts id
 * @see Concert::setId
 */
QString Concert::id() const
{
    return m_id;
}

/**
 * @brief The stream details object of this concert
 * @return StreamDetails Object
 */
StreamDetails *Concert::streamDetails() const
{
    return m_streamDetails;
}

/**
 * @brief Concert::nfoContent
 * @return
 */
QString Concert::nfoContent() const
{
    return m_nfoContent;
}

/**
 * @brief Concert::databaseId
 * @return
 */
int Concert::databaseId() const
{
    return m_databaseId;
}

bool Concert::syncNeeded() const
{
    return m_syncNeeded;
}

QStringList Concert::tags() const
{
    return m_tags;
}

/*** SETTER ***/

/**
 * @brief Sets the concerts name
 * @param name Name of the concert
 * @see Concert::name
 */
void Concert::setName(QString name)
{
    m_name = name;
    setChanged(true);
}

/**
 * @brief Sets the concerts artist
 * @param artist Artist of the concert
 * @see Concert::artist
 */
void Concert::setArtist(QString artist)
{
    m_artist = artist;
    setChanged(true);
}

/**
 * @brief Sets the concerts album
 * @param album Album of the concert
 * @see Concert::album
 */
void Concert::setAlbum(QString album)
{
    m_album = album;
    setChanged(true);
}

/**
 * @brief Sets the concerts plot
 * @param overview Plot of the concert
 * @see Concert::overview
 */
void Concert::setOverview(QString overview)
{
    m_overview = overview;
    setChanged(true);
}

/**
 * @brief Sets the concerts rating
 * @param rating Rating of the concert
 * @see Concert::rating
 */
void Concert::setRating(qreal rating)
{
    m_rating = rating;
    setChanged(true);
}

/**
 * @brief Sets the concerts release date
 * @param released Release date of the concert
 * @see Concert::released
 */
void Concert::setReleased(QDate released)
{
    m_released = released;
    setChanged(true);
}

/**
 * @brief Sets the concerts tagline
 * @param tagline Tagline of the concert
 * @see Concert::tagline
 */
void Concert::setTagline(QString tagline)
{
    m_tagline = tagline;
    setChanged(true);
}

/**
 * @brief Sets the concerts runtime
 * @param runtime Runtime in minutes
 * @see Concert::runtime
 */
void Concert::setRuntime(std::chrono::minutes runtime)
{
    m_runtime = runtime;
    setChanged(true);
}

/**
 * @brief Sets the concerts certification
 * @param certification Certification of the concert
 * @see Concert::certification
 */
void Concert::setCertification(QString certification)
{
    m_certification = certification;
    setChanged(true);
}

/**
 * @brief Sets the concerts trailer
 * @param trailer URL of the concerts trailer
 * @see Concert::trailer
 */
void Concert::setTrailer(QUrl trailer)
{
    m_trailer = trailer;
    setChanged(true);
}

/**
 * @brief Sets the concerts playcount
 * @param playcount Playcount of the concert
 * @see Concert::playcount
 */
void Concert::setPlayCount(int playcount)
{
    m_playcount = playcount;
    setChanged(true);
}

/**
 * @brief Sets the concerts last playtime. If the concert has never played, set an invalid date.
 * @param lastPlayed Last playtime of the concert
 * @see Concert::lastPlayed
 */
void Concert::setLastPlayed(QDateTime lastPlayed)
{
    m_lastPlayed = lastPlayed;
    setChanged(true);
}

/**
 * @brief Sets the concerts posters
 * @param posters List of poster
 * @see Concert::posters
 */
void Concert::setPosters(QList<Poster> posters)
{
    m_posters = posters;
    setChanged(true);
}

/**
 * @brief Sets a specific concert poster
 * @param index Index of the position in the poster list
 * @param poster Poster to set
 * @see Concert::posters
 */
void Concert::setPoster(int index, Poster poster)
{
    if (m_posters.size() < index) {
        return;
    }
    m_posters[index] = poster;
    setChanged(true);
}

/**
 * @brief Sets the concert backdrops
 * @param backdrops List of backdrops
 * @see Concert::backdrops
 */
void Concert::setBackdrops(QList<Poster> backdrops)
{
    m_backdrops.append(backdrops);
    setChanged(true);
}

/**
 * @brief Sets a specific concert backdrop
 * @param index Index of the position in the backdrop list
 * @param backdrop Backdrop to set
 * @see Concert::backdrops
 */
void Concert::setBackdrop(int index, Poster backdrop)
{
    if (m_backdrops.size() < index) {
        return;
    }
    m_backdrops[index] = backdrop;
    setChanged(true);
}

/**
 * @brief Sets the concerts watched status
 * @param watched Watched status of the concert
 * @see Concert::watched
 */
void Concert::setWatched(bool watched)
{
    m_watched = watched;
    setChanged(true);
}

/**
 * @brief Sets if some of the concerts info has changed. Emits the sigChanged signal
 * @param changed Infos have changed
 * @see Concert::hasChanged
 */
void Concert::setChanged(bool changed)
{
    m_hasChanged = changed;
    emit sigChanged(this);
}

/**
 * @brief Sets if downloads are in progress
 * @param inProgress Status of downloads
 */
void Concert::setDownloadsInProgress(bool inProgress)
{
    m_downloadsInProgress = inProgress;
}

/**
 * @brief Sets the number of downloads left
 * @param downloadsLeft Number of downloads left
 */
void Concert::setDownloadsSize(int downloadsLeft)
{
    m_downloadsSize = downloadsLeft;
}

/**
 * @brief Sets if the concert files are stored in a separate folder
 * @param inSepFolder Files of the concert are in one separate folder
 */
void Concert::setInSeparateFolder(bool inSepFolder)
{
    m_inSeparateFolder = inSepFolder;
}

/**
 * @brief Sets the media center id of the concert
 * @param mediaCenterId Id of the concert
 */
void Concert::setMediaCenterId(int mediaCenterId)
{
    m_mediaCenterId = mediaCenterId;
}

/**
 * @brief Sets the concerts tmdb id
 * @param id Tmdb id of the concert
 * @see Concert::tmdbId
 */
void Concert::setTmdbId(QString id)
{
    m_tmdbId = id;
    setChanged(true);
}

/**
 * @brief Sets the concerts id
 * @param id Imdb id of the concert
 * @see Concert::id
 */
void Concert::setId(QString id)
{
    m_id = id;
    setChanged(true);
}

/**
 * @brief Sets if the stream details were loaded
 * @param loaded
 * @see Concert::streamDetailsLoaded
 */
void Concert::setStreamDetailsLoaded(bool loaded)
{
    m_streamDetailsLoaded = loaded;
}

/**
 * @brief Concert::setNfoContent
 * @param content
 */
void Concert::setNfoContent(QString content)
{
    m_nfoContent = content;
}

/**
 * @brief Concert::setDatabaseId
 * @param id
 */
void Concert::setDatabaseId(int id)
{
    m_databaseId = id;
}

/*** ADDER ***/

/**
 * @brief Adds a genre to the concert
 * @param genre Genre to add
 * @see Concert::genres
 */
void Concert::addGenre(QString genre)
{
    if (genre.isEmpty()) {
        return;
    }
    m_genres.append(genre);
    setChanged(true);
}

void Concert::addTag(QString tag)
{
    m_tags.append(tag);
    setChanged(true);
}

/**
 * @brief Adds a poster to the concert
 * @param poster Poster to add
 * @see Concert::posters
 */
void Concert::addPoster(Poster poster)
{
    m_posters.append(poster);
    setChanged(true);
}

/**
 * @brief Adds a backdrop to the concert
 * @param backdrop Backdrop to add
 * @see Concert::backdrops
 */
void Concert::addBackdrop(Poster backdrop)
{
    m_backdrops.append(backdrop);
    setChanged(true);
}

void Concert::setSyncNeeded(bool syncNeeded)
{
    m_syncNeeded = syncNeeded;
}

/*** REMOVER ***/

/**
 * @brief Concert::removeGenre
 * @param genre
 * @see Concert::genres
 */
void Concert::removeGenre(QString genre)
{
    m_genres.removeAll(genre);
    setChanged(true);
}

void Concert::removeTag(QString tag)
{
    m_tags.removeAll(tag);
    setChanged(true);
}

void Concert::addExtraFanart(QByteArray fanart)
{
    m_extraFanartImagesToAdd.append(fanart);
    setChanged(true);
}

void Concert::removeExtraFanart(QByteArray fanart)
{
    m_extraFanartImagesToAdd.removeOne(fanart);
    setChanged(true);
}

void Concert::removeExtraFanart(QString file)
{
    m_extraFanarts.removeOne(file);
    m_extraFanartsToRemove.append(file);
    setChanged(true);
}

QList<ExtraFanart> Concert::extraFanarts(MediaCenterInterface *mediaCenterInterface)
{
    if (m_extraFanarts.isEmpty()) {
        m_extraFanarts = mediaCenterInterface->extraFanartNames(this);
    }
    foreach (const QString &file, m_extraFanartsToRemove) {
        m_extraFanarts.removeOne(file);
    }
    QList<ExtraFanart> fanarts;
    foreach (const QString &file, m_extraFanarts) {
        ExtraFanart f;
        f.path = file;
        fanarts.append(f);
    }
    foreach (const QByteArray &img, m_extraFanartImagesToAdd) {
        ExtraFanart f;
        f.image = img;
        fanarts.append(f);
    }
    return fanarts;
}

QStringList Concert::extraFanartsToRemove()
{
    return m_extraFanartsToRemove;
}

QList<QByteArray> Concert::extraFanartImagesToAdd()
{
    return m_extraFanartImagesToAdd;
}

void Concert::clearExtraFanartData()
{
    m_extraFanartImagesToAdd.clear();
    m_extraFanartsToRemove.clear();
    m_extraFanarts.clear();
}

DiscType Concert::discType() const
{
    if (files().isEmpty()) {
        return DiscType::Single;
    }
    if (Helper::instance()->isDvd(files().first())) {
        return DiscType::Dvd;
    }
    if (Helper::instance()->isBluRay(files().first())) {
        return DiscType::BluRay;
    }
    return DiscType::Single;
}

QList<ImageType> Concert::imagesToRemove() const
{
    return m_imagesToRemove;
}

void Concert::removeImage(ImageType type)
{
    if (!m_images.value(type, QByteArray()).isNull()) {
        m_images.insert(type, QByteArray());
        m_hasImageChanged.insert(type, false);
    } else if (!m_imagesToRemove.contains(type)) {
        m_imagesToRemove.append(type);
    }
    setChanged(true);
}

bool Concert::lessThan(Concert *a, Concert *b)
{
    return (QString::localeAwareCompare(
                Helper::instance()->appendArticle(a->name()), Helper::instance()->appendArticle(b->name()))
            < 0);
}

QList<ImageType> Concert::imageTypes()
{
    return {ImageType::ConcertPoster,
        ImageType::ConcertCdArt,
        ImageType::ConcertClearArt,
        ImageType::ConcertLogo,
        ImageType::ConcertBackdrop};
}

QByteArray Concert::image(ImageType imageType)
{
    return m_images.value(imageType, QByteArray());
}

bool Concert::imageHasChanged(ImageType imageType)
{
    return m_hasImageChanged.value(imageType, false);
}

void Concert::setImage(ImageType imageType, QByteArray image)
{
    m_images.insert(imageType, image);
    m_hasImageChanged.insert(imageType, true);
    setChanged(true);
}

bool Concert::hasImage(ImageType imageType)
{
    return m_hasImage.value(imageType, false);
}

void Concert::setHasImage(ImageType imageType, bool has)
{
    m_hasImage.insert(imageType, has);
}

void Concert::setHasExtraFanarts(bool has)
{
    m_hasExtraFanarts = has;
}

bool Concert::hasExtraFanarts() const
{
    return m_hasExtraFanarts;
}
