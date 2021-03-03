#include "Concert.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "data/StreamDetails.h"
#include "file/NameFormatter.h"
#include "globals/Helper.h"
#include "media_centers/MediaCenterInterface.h"
#include "settings/Settings.h"

using namespace std::chrono_literals;

/**
 * \brief Constructs a new concert object
 * \param files List of files for this concert
 */
Concert::Concert(const mediaelch::FileList& files, QObject* parent) :
    QObject(parent),
    m_controller{new ConcertController(this)},

    m_hasChanged{false},

    m_inSeparateFolder{false},
    m_streamDetailsLoaded{false},
    m_syncNeeded{false},
    m_hasExtraFanarts{false}
{
    moveToThread(QApplication::instance()->thread());
    static int s_idCounter = 0;
    m_concert.concertId = ++s_idCounter;
    setFiles(files);
}

void Concert::setFiles(const mediaelch::FileList& files)
{
    m_files = files;
    m_concert.streamDetails = new StreamDetails(this, files);
    if (!files.isEmpty()) {
        QFileInfo fi(files.at(0).toString());
        QStringList path = fi.path().split("/", ElchSplitBehavior::SkipEmptyParts);
        m_folderName = path.last();
    }
}

/**
 * \brief Clears all infos in the concert
 */
void Concert::clear()
{
    QSet<ConcertScraperInfo> infos;
    infos.reserve(14);
    infos << ConcertScraperInfo::Title         //
          << ConcertScraperInfo::Tagline       //
          << ConcertScraperInfo::Rating        //
          << ConcertScraperInfo::Released      //
          << ConcertScraperInfo::Runtime       //
          << ConcertScraperInfo::Certification //
          << ConcertScraperInfo::Trailer       //
          << ConcertScraperInfo::Overview      //
          << ConcertScraperInfo::Poster        //
          << ConcertScraperInfo::Backdrop      //
          << ConcertScraperInfo::Genres        //
          << ConcertScraperInfo::Tags          //
          << ConcertScraperInfo::ExtraArts     //
          << ConcertScraperInfo::ExtraFanarts;
    clear(infos);
    m_nfoContent.clear();
}

/// \brief Clears contents of the concert based on a list
/// \param infos List of infos which should be cleared
void Concert::clear(QSet<ConcertScraperInfo> infos)
{
    if (infos.contains(ConcertScraperInfo::Backdrop)) {
        m_concert.backdrops.clear();
        m_concert.images.insert(ImageType::ConcertBackdrop, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertBackdrop, false);
        m_imagesToRemove.removeOne(ImageType::ConcertBackdrop);
    }
    if (infos.contains(ConcertScraperInfo::Genres)) {
        m_concert.genres.clear();
    }
    if (infos.contains(ConcertScraperInfo::Poster)) {
        m_concert.posters.clear();
        m_concert.images.insert(ImageType::ConcertPoster, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertPoster, false);
        m_imagesToRemove.removeOne(ImageType::ConcertPoster);
    }
    if (infos.contains(ConcertScraperInfo::Overview)) {
        m_concert.overview = "";
    }
    if (infos.contains(ConcertScraperInfo::Rating)) {
        m_concert.ratings.clear();
        m_concert.ratings.push_back(Rating{});
        m_concert.userRating = 0.0;
    }
    if (infos.contains(ConcertScraperInfo::Released)) {
        m_concert.releaseDate = QDate(2000, 02, 30); // invalid date
    }
    if (infos.contains(ConcertScraperInfo::Tagline)) {
        m_concert.tagline = "";
    }
    if (infos.contains(ConcertScraperInfo::Runtime)) {
        m_concert.runtime = 0min;
    }
    if (infos.contains(ConcertScraperInfo::Trailer)) {
        m_concert.trailer = "";
    }
    if (infos.contains(ConcertScraperInfo::Certification)) {
        m_concert.certification = Certification::NoCertification;
    }
    if (infos.contains(ConcertScraperInfo::Tags)) {
        m_concert.tags.clear();
    }
    if (infos.contains(ConcertScraperInfo::ExtraArts)) {
        m_concert.images.insert(ImageType::ConcertCdArt, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertCdArt, false);
        m_concert.images.insert(ImageType::ConcertLogo, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertLogo, false);
        m_concert.images.insert(ImageType::ConcertClearArt, QByteArray());
        m_hasImageChanged.insert(ImageType::ConcertClearArt, false);
        m_imagesToRemove.removeOne(ImageType::ConcertCdArt);
        m_imagesToRemove.removeOne(ImageType::ConcertClearArt);
        m_imagesToRemove.removeOne(ImageType::ConcertLogo);
    }
    if (infos.contains(ConcertScraperInfo::ExtraFanarts)) {
        m_extraFanartsToRemove.clear();
        m_extraFanartImagesToAdd.clear();
        m_concert.extraFanarts.clear();
    }
}

QString Concert::originalTitle() const
{
    return m_concert.originalTitle;
}

ConcertController* Concert::controller() const
{
    return m_controller;
}

/**
 * \brief Clears the concert images to save memory
 */
void Concert::clearImages()
{
    m_concert.images.clear();
    m_hasImageChanged.clear();
    m_extraFanartImagesToAdd.clear();
}

/*** GETTER ***/

QString Concert::title() const
{
    return m_concert.title;
}

/**
 * \property Concert::artist
 * \brief Holds the concerts artist
 * \return The concerts artist
 * \see Concert::setArtist
 */
QString Concert::artist() const
{
    return m_concert.artist;
}

/**
 * \property Concert::album
 * \brief Holds the concerts album
 * \return The concerts album
 * \see Concert::setAlbum
 */
QString Concert::album() const
{
    return m_concert.album;
}

/**
 * \property Concert::overview
 * \brief Holds the concerts plot
 * \return Plot of the concert
 * \see Concert::setOverview
 */
QString Concert::overview() const
{
    return m_concert.overview;
}

QVector<Rating>& Concert::ratings()
{
    return m_concert.ratings;
}

const QVector<Rating>& Concert::ratings() const
{
    return m_concert.ratings;
}

double Concert::userRating() const
{
    return m_concert.userRating;
}

/**
 * \property Concert::released
 * \brief Holds the concerts release date
 * \return Release date of the concert
 * \see Concert::setReleased
 */
QDate Concert::released() const
{
    return m_concert.releaseDate;
}

/**
 * \property Concert::tagline
 * \brief Holds the concerts tagline
 * \return Tagline of the concert
 * \see Concert::setTagline
 */
QString Concert::tagline() const
{
    return m_concert.tagline;
}

/**
 * \brief Holds the concerts runtime
 * \return Runtime of the concert
 * \see Concert::setRuntime
 */
std::chrono::minutes Concert::runtime() const
{
    return m_concert.runtime;
}

/**
 * \property Concert::certification
 * \brief Holds the concerts certification
 * \return Certification of the concert
 * \see Concert::setCertification
 */
Certification Concert::certification() const
{
    return m_concert.certification;
}

/**
 * \property Concert::genres
 * \brief Holds a list of the concert genres
 * \return List of genres of the concert
 * \see Concert::setGenres
 * \see Concert::genresPointer
 * \see Concert::addGenre
 * \see Concert::removeGenre
 */
QStringList Concert::genres() const
{
    return m_concert.genres;
}

/**
 * \brief Returns a list of pointers to QStrings
 * \return List of pointers to the concert genres
 */
QVector<QString*> Concert::genresPointer()
{
    QVector<QString*> genres;
    for (int i = 0, n = m_concert.genres.size(); i < n; ++i) {
        genres.append(&m_concert.genres[i]);
    }
    return genres;
}

/**
 * \property Concert::trailer
 * \brief Holds the concerts trailer
 * \return Trailer of the concert
 * \see Concert::setTrailer
 */
QUrl Concert::trailer() const
{
    return m_concert.trailer;
}

/**
 * \brief Holds the files of the concert
 * \return List of files
 */
const mediaelch::FileList& Concert::files() const
{
    return m_files;
}

/**
 * \property Concert::playcount
 * \brief Holds the playcount
 * \return Playcount of the concert
 * \see Concert::setPlayCount
 */
int Concert::playcount() const
{
    return m_concert.playcount;
}

/**
 * \property Concert::lastPlayed
 * \brief Holds the date when the concert was last played
 *        If the concert was never played an invalid date will be returned
 * \return Date of last playtime
 * \see Concert::setLastPlayed
 */
QDateTime Concert::lastPlayed() const
{
    return m_concert.lastPlayed;
}

/**
 * \property Concert::posters
 * \brief Holds a list of posters of the concert
 * \return List of posters
 * \see Concert::setPosters
 * \see Concert::setPoster
 * \see Concert::addPoster
 */
QVector<Poster> Concert::posters() const
{
    return m_concert.posters;
}

/**
 * \property Concert::backdrops
 * \brief Holds a list of backdrops of the concert
 * \return List of backdrops
 * \see Concert::setBackdrops
 * \see Concert::setBackdrop
 * \see Concert::addBackdrop
 */
QVector<Poster> Concert::backdrops() const
{
    return m_concert.backdrops;
}

/**
 * \brief Returns the parent folder of the concert
 * \return Parent folder of the concert
 */
QString Concert::folderName() const
{
    return m_folderName;
}

/**
 * \property Concert::streamDetailsLoaded
 * \brief Holds if the stream details were loaded
 * \return True if the stream details were loaded
 * \see Concert::setStreamDetailsLoaded
 */
bool Concert::streamDetailsLoaded() const
{
    return m_streamDetailsLoaded;
}

bool Concert::watched() const
{
    return m_concert.playcount > 0;
}

/**
 * \property Concert::hasChanged
 * \brief Holds a property if the concert infos were changed by a setter or a ScraperInterface
 * \return True if some of the concert infos were changed
 * \see Concert::setChanged
 */
bool Concert::hasChanged() const
{
    return m_hasChanged;
}

/**
 * \brief Holds a unique MediaElch concert id
 * \return MediaElchs id of the concert
 */
int Concert::concertId() const
{
    return m_concert.concertId;
}

/**
 * \brief Returns true if a download is in progress
 * \return Download is in progress
 */
bool Concert::downloadsInProgress() const
{
    return m_downloadsInProgress;
}

/**
 * \brief Returns how many downloads are left for this concert
 * \return Number of downloads left
 */
int Concert::downloadsSize() const
{
    return m_downloadsSize;
}

/**
 * \brief Holds if the concert files are stored in a separate folder
 * \return Concert files are stored in a separate folder
 */
bool Concert::inSeparateFolder() const
{
    return m_inSeparateFolder;
}

/**
 * \brief Concert::mediaCenterId
 * \return Id in a MediaCenterInterface
 */
int Concert::mediaCenterId() const
{
    return m_concert.mediaCenterId;
}

/**
 * \property Concert::tmdbId
 * \brief Holds the concerts tmdb id
 * \return The concerts tmdb id
 * \see Concert::setTmdbId
 */
TmdbId Concert::tmdbId() const
{
    return m_concert.tmdbId;
}

/**
 * \property Concert::id
 * \brief Holds the concerts id
 * \return The concerts id
 * \see Concert::setId
 */
ImdbId Concert::imdbId() const
{
    return m_concert.imdbId;
}

/**
 * \brief The stream details object of this concert
 * \return StreamDetails Object
 */
StreamDetails* Concert::streamDetails() const
{
    return m_concert.streamDetails;
}

QString Concert::nfoContent() const
{
    return m_nfoContent;
}

int Concert::databaseId() const
{
    return m_concert.databaseId;
}

bool Concert::syncNeeded() const
{
    return m_syncNeeded;
}

QStringList Concert::tags() const
{
    return m_concert.tags;
}

/*** SETTER ***/

void Concert::setTitle(QString title)
{
    m_concert.title = std::move(title);
    setChanged(true);
}

void Concert::setOriginalTitle(QString title)
{
    m_concert.originalTitle = std::move(title);
    setChanged(true);
}

/**
 * \brief Sets the concerts artist
 * \param artist Artist of the concert
 * \see Concert::artist
 */
void Concert::setArtist(QString artist)
{
    m_concert.artist = std::move(artist);
    setChanged(true);
}

/**
 * \brief Sets the concerts album
 * \param album Album of the concert
 * \see Concert::album
 */
void Concert::setAlbum(QString album)
{
    m_concert.album = std::move(album);
    setChanged(true);
}

/**
 * \brief Sets the concerts plot
 * \param overview Plot of the concert
 * \see Concert::overview
 */
void Concert::setOverview(QString overview)
{
    m_concert.overview = std::move(overview);
    setChanged(true);
}

void Concert::setUserRating(double userRating)
{
    m_concert.userRating = userRating;
    setChanged(true);
}

/**
 * \brief Sets the concerts release date
 * \param released Release date of the concert
 * \see Concert::released
 */
void Concert::setReleased(QDate released)
{
    m_concert.releaseDate = released;
    setChanged(true);
}

/**
 * \brief Sets the concerts tagline
 * \param tagline Tagline of the concert
 * \see Concert::tagline
 */
void Concert::setTagline(QString tagline)
{
    m_concert.tagline = std::move(tagline);
    setChanged(true);
}

/**
 * \brief Sets the concerts runtime
 * \param runtime Runtime in minutes
 * \see Concert::runtime
 */
void Concert::setRuntime(std::chrono::minutes runtime)
{
    m_concert.runtime = runtime;
    setChanged(true);
}

/**
 * \brief Sets the concerts certification
 * \param cert Certification of the concert
 * \see Concert::certification
 */
void Concert::setCertification(Certification cert)
{
    m_concert.certification = std::move(cert);
    setChanged(true);
}

/**
 * \brief Sets the concerts trailer
 * \param trailer URL of the concerts trailer
 * \see Concert::trailer
 */
void Concert::setTrailer(QUrl trailer)
{
    m_concert.trailer = std::move(trailer);
    setChanged(true);
}

/**
 * \brief Sets the concerts playcount
 * \param playcount Playcount of the concert
 * \see Concert::playcount
 */
void Concert::setPlayCount(int playcount)
{
    m_concert.playcount = playcount;
    setChanged(true);
}

/**
 * \brief Sets the concerts last playtime. If the concert has never played, set an invalid date.
 * \param lastPlayed Last playtime of the concert
 * \see Concert::lastPlayed
 */
void Concert::setLastPlayed(QDateTime lastPlayed)
{
    m_concert.lastPlayed = std::move(lastPlayed);
    setChanged(true);
}

/**
 * \brief Sets the concerts posters
 * \param posters List of poster
 * \see Concert::posters
 */
void Concert::setPosters(QVector<Poster> posters)
{
    m_concert.posters = std::move(posters);
    setChanged(true);
}

/**
 * \brief Sets a specific concert poster
 * \param index Index of the position in the poster list
 * \param poster Poster to set
 * \see Concert::posters
 */
void Concert::setPoster(int index, Poster poster)
{
    if (m_concert.posters.size() < index) {
        return;
    }
    m_concert.posters[index] = std::move(poster);
    setChanged(true);
}

/**
 * \brief Sets the concert backdrops
 * \param backdrops List of backdrops
 * \see Concert::backdrops
 */
void Concert::setBackdrops(QVector<Poster> backdrops)
{
    m_concert.backdrops.append(backdrops);
    setChanged(true);
}

/**
 * \brief Sets a specific concert backdrop
 * \param index Index of the position in the backdrop list
 * \param backdrop Backdrop to set
 * \see Concert::backdrops
 */
void Concert::setBackdrop(int index, Poster backdrop)
{
    if (m_concert.backdrops.size() < index) {
        return;
    }
    m_concert.backdrops[index] = std::move(backdrop);
    setChanged(true);
}

/**
 * \brief Sets if some of the concerts info has changed. Emits the sigChanged signal
 * \param changed Infos have changed
 * \see Concert::hasChanged
 */
void Concert::setChanged(bool changed)
{
    m_hasChanged = changed;
    emit sigChanged(this);
}

/**
 * \brief Sets if downloads are in progress
 * \param inProgress Status of downloads
 */
void Concert::setDownloadsInProgress(bool inProgress)
{
    m_downloadsInProgress = inProgress;
}

/**
 * \brief Sets the number of downloads left
 * \param downloadsLeft Number of downloads left
 */
void Concert::setDownloadsSize(int downloadsLeft)
{
    m_downloadsSize = downloadsLeft;
}

/**
 * \brief Sets if the concert files are stored in a separate folder
 * \param inSepFolder Files of the concert are in one separate folder
 */
void Concert::setInSeparateFolder(bool inSepFolder)
{
    m_inSeparateFolder = inSepFolder;
}

/**
 * \brief Sets the media center id of the concert
 * \param mediaCenterId Id of the concert
 */
void Concert::setMediaCenterId(int mediaCenterId)
{
    m_concert.mediaCenterId = mediaCenterId;
}

/**
 * \brief Sets the concerts tmdb id
 * \param id Tmdb id of the concert
 * \see Concert::tmdbId
 */
void Concert::setTmdbId(TmdbId id)
{
    m_concert.tmdbId = std::move(id);
    setChanged(true);
}

/**
 * \brief Sets the concerts id
 * \param id Imdb id of the concert
 * \see Concert::id
 */
void Concert::setImdbId(ImdbId id)
{
    m_concert.imdbId = std::move(id);
    setChanged(true);
}

/**
 * \brief Sets if the stream details were loaded
 * \see Concert::streamDetailsLoaded
 */
void Concert::setStreamDetailsLoaded(bool loaded)
{
    m_streamDetailsLoaded = loaded;
}

/**
 * \brief Concert::setNfoContent
 */
void Concert::setNfoContent(QString content)
{
    m_nfoContent = std::move(content);
}

/**
 * \brief Concert::setDatabaseId
 */
void Concert::setDatabaseId(int id)
{
    m_concert.databaseId = id;
}

/*** ADDER ***/

/**
 * \brief Adds a genre to the concert
 * \param genre Genre to add
 * \see Concert::genres
 */
void Concert::addGenre(QString genre)
{
    if (genre.isEmpty()) {
        return;
    }
    m_concert.genres.append(genre);
    setChanged(true);
}

void Concert::addTag(QString tag)
{
    m_concert.tags.append(tag);
    setChanged(true);
}

/**
 * \brief Adds a poster to the concert
 * \param poster Poster to add
 * \see Concert::posters
 */
void Concert::addPoster(Poster poster)
{
    m_concert.posters.append(poster);
    setChanged(true);
}

/**
 * \brief Adds a backdrop to the concert
 * \param backdrop Backdrop to add
 * \see Concert::backdrops
 */
void Concert::addBackdrop(Poster backdrop)
{
    m_concert.backdrops.append(backdrop);
    setChanged(true);
}

void Concert::setSyncNeeded(bool syncNeeded)
{
    m_syncNeeded = syncNeeded;
}

/*** REMOVER ***/

/**
 * \brief Concert::removeGenre
 * \see Concert::genres
 */
void Concert::removeGenre(QString genre)
{
    m_concert.genres.removeAll(genre);
    setChanged(true);
}

void Concert::removeTag(QString tag)
{
    m_concert.tags.removeAll(tag);
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
    m_concert.extraFanarts.removeOne(file);
    m_extraFanartsToRemove.append(file);
    setChanged(true);
}

QVector<ExtraFanart> Concert::extraFanarts(MediaCenterInterface* mediaCenterInterface)
{
    if (m_concert.extraFanarts.isEmpty()) {
        m_concert.extraFanarts = mediaCenterInterface->extraFanartNames(this);
    }
    for (const QString& file : asConst(m_extraFanartsToRemove)) {
        m_concert.extraFanarts.removeOne(file);
    }
    QVector<ExtraFanart> fanarts;
    for (const QString& file : asConst(m_concert.extraFanarts)) {
        ExtraFanart f;
        f.path = file;
        fanarts.append(f);
    }
    for (const QByteArray& img : asConst(m_extraFanartImagesToAdd)) {
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

QVector<QByteArray> Concert::extraFanartImagesToAdd()
{
    return m_extraFanartImagesToAdd;
}

void Concert::clearExtraFanartData()
{
    m_extraFanartImagesToAdd.clear();
    m_extraFanartsToRemove.clear();
    m_concert.extraFanarts.clear();
}

DiscType Concert::discType() const
{
    if (files().isEmpty()) {
        return DiscType::Single;
    }
    if (helper::isDvd(files().first())) {
        return DiscType::Dvd;
    }
    if (helper::isBluRay(files().first())) {
        return DiscType::BluRay;
    }
    return DiscType::Single;
}

QVector<ImageType> Concert::imagesToRemove() const
{
    return m_imagesToRemove;
}

void Concert::removeImage(ImageType type)
{
    if (!m_concert.images.value(type, QByteArray()).isNull()) {
        m_concert.images.insert(type, QByteArray());
        m_hasImageChanged.insert(type, false);
    } else if (!m_imagesToRemove.contains(type)) {
        m_imagesToRemove.append(type);
    }
    setChanged(true);
}

bool Concert::lessThan(Concert* a, Concert* b)
{
    return (QString::localeAwareCompare(helper::appendArticle(a->title()), helper::appendArticle(b->title())) < 0);
}

QVector<ImageType> Concert::imageTypes()
{
    return {ImageType::ConcertPoster,
        ImageType::ConcertCdArt,
        ImageType::ConcertClearArt,
        ImageType::ConcertLogo,
        ImageType::ConcertBackdrop};
}

QByteArray Concert::image(ImageType imageType) const
{
    return m_concert.images.value(imageType, QByteArray());
}

bool Concert::imageHasChanged(ImageType imageType)
{
    return m_hasImageChanged.value(imageType, false);
}

void Concert::setImage(ImageType imageType, QByteArray image)
{
    m_concert.images.insert(imageType, image);
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
