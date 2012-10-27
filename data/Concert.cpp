#include "Concert.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "globals/NameFormatter.h"

/**
 * @brief Constructs a new concert object
 * @param files List of files for this concert
 * @param parent
 */
Concert::Concert(QStringList files, QObject *parent) :
    QObject(parent)
{
    moveToThread(QApplication::instance()->thread());
    m_files = files;
    m_rating = 0;
    m_runtime = 0;
    m_playcount = 0;
    m_backdropImageChanged = false;
    m_posterImageChanged = false;
    m_logoImageChanged = false;
    m_clearArtImageChanged = false;
    m_cdArtImageChanged = false;
    if (files.size() > 0) {
        QFileInfo fi(files.at(0));
        QStringList path = fi.path().split("/", QString::SkipEmptyParts);
        if (!path.isEmpty())
            m_folderName = path.last();
    }
    m_infoLoaded = false;
    m_watched = false;
    m_hasChanged = false;
    m_downloadsInProgress = false;
    m_downloadsSize = 0;
    m_inSeparateFolder = false;
    static int m_idCounter = 0;
    m_concertId = ++m_idCounter;
    m_mediaCenterId = -1;
    m_streamDetailsLoaded = false;
    if (!files.isEmpty())
        m_streamDetails = new StreamDetails(this, files.at(0));
    else
        m_streamDetails = new StreamDetails(this, "");
}

Concert::~Concert()
{
}

/**
 * @brief Clears all infos in the concert
 */
void Concert::clear()
{
    QList<int> infos;
    infos << ConcertScraperInfos::Title
          << ConcertScraperInfos::Tagline
          << ConcertScraperInfos::Rating
          << ConcertScraperInfos::Released
          << ConcertScraperInfos::Runtime
          << ConcertScraperInfos::Certification
          << ConcertScraperInfos::Trailer
          << ConcertScraperInfos::Overview
          << ConcertScraperInfos::Poster
          << ConcertScraperInfos::Backdrop
          << ConcertScraperInfos::Genres;
    clear(infos);
}

/**
 * @brief Clears contents of the concert based on a list
 * @param infos List of infos which should be cleared
 */
void Concert::clear(QList<int> infos)
{
    if (infos.contains(ConcertScraperInfos::Backdrop))
        m_backdrops.clear();
    if (infos.contains(ConcertScraperInfos::Genres))
        m_genres.clear();
    if (infos.contains(ConcertScraperInfos::Poster))
        m_posters.clear();
    if (infos.contains(ConcertScraperInfos::Overview))
        m_overview = "";
    if (infos.contains(ConcertScraperInfos::Rating))
        m_rating = 0;
    if (infos.contains(ConcertScraperInfos::Released))
        m_released = QDate(2000, 02, 30); // invalid date
    if (infos.contains(ConcertScraperInfos::Tagline))
        m_tagline = "";
    if (infos.contains(ConcertScraperInfos::Runtime))
        m_runtime = 0;
    if (infos.contains(ConcertScraperInfos::Trailer))
        m_trailer = "";
    if (infos.contains(ConcertScraperInfos::Certification))
        m_certification = "";
}

/**
 * @brief Saves the concert infos with the given MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use for saving
 * @return Saving was successful or not
 */
bool Concert::saveData(MediaCenterInterface *mediaCenterInterface)
{
    qDebug() << "Entered";
    if (!streamDetailsLoaded())
        loadStreamDetailsFromFile();
    bool saved = mediaCenterInterface->saveConcert(this);
    qDebug() << "Saved" << saved;
    if (!m_infoLoaded)
        m_infoLoaded = saved;
    setChanged(false);
    clearImages();
    return saved;
}

/**
 * @brief Loads the concert infos with the given MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use for loading
 * @param force Force the loading. If set to false and infos were already loeaded this function just returns
 * @return Loading was successful or not
 */
bool Concert::loadData(MediaCenterInterface *mediaCenterInterface, bool force)
{
    if ((m_infoLoaded || hasChanged()) && !force)
        return m_infoLoaded;

    bool infoLoaded = mediaCenterInterface->loadConcert(this);
    if (!infoLoaded) {
        NameFormatter *nameFormat = NameFormatter::instance(this);
        if (this->files().size() > 0) {
            QFileInfo fi(this->files().at(0));
            if (QString::compare(fi.fileName(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (pathElements.size() > 0 && QString::compare(pathElements.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0)
                    pathElements.removeLast();
                if (pathElements.size() > 0)
                    this->setName(pathElements.last());
            } else if (QString::compare(fi.fileName(), "index.bdmv", Qt::CaseInsensitive) == 0) {
                    QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                    if (pathElements.size() > 0 && QString::compare(pathElements.last(), "BDMV", Qt::CaseInsensitive) == 0)
                        pathElements.removeLast();
                    if (pathElements.size() > 0)
                        this->setName(pathElements.last());
            } else if (inSeparateFolder()) {
                QStringList splitted = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!splitted.isEmpty()) {
                    setName(nameFormat->formatName(splitted.last()));
                } else {
                    if (files().size() > 1)
                        setName(nameFormat->formatName(nameFormat->formatParts(fi.completeBaseName())));
                    else
                        setName(nameFormat->formatName(fi.completeBaseName()));
                }
            } else {
                if (files().size() > 1)
                    setName(nameFormat->formatName(nameFormat->formatParts(fi.completeBaseName())));
                else
                    setName(nameFormat->formatName(fi.completeBaseName()));
            }


        }
    }
    m_infoLoaded = infoLoaded;
    setChanged(false);
    return infoLoaded;
}

/**
 * @brief Loads the concert info from a scraper
 * @param id Id of the concert within the given ScraperInterface
 * @param scraperInterface ScraperInterface to use for loading
 * @param infos List of infos to load
 */
void Concert::loadData(QString id, ConcertScraperInterface *scraperInterface, QList<int> infos)
{
    qDebug() << "Entered, id=" << id << "scraperInterface=" << scraperInterface->name();
    m_infosToLoad = infos;
    if (scraperInterface->name() == "The Movie DB (Concerts)")
        setTmdbId(id);
    scraperInterface->loadData(id, this, infos);
}

/**
 * @brief Tries to load streamdetails from the file
 */
void Concert::loadStreamDetailsFromFile()
{
    m_streamDetails->loadStreamDetails();
    setStreamDetailsLoaded(true);
}

/**
 * @brief Concert::infosToLoad
 * @return
 */
QList<int> Concert::infosToLoad()
{
    return m_infosToLoad;
}

/**
 * @brief Called when a ScraperInterface has finished loading
 *        Emits the loaded signal
 */
void Concert::scraperLoadDone()
{
    emit loaded(this);
}

/**
 * @brief Clears the concert images to save memory
 */
void Concert::clearImages()
{
    m_posterImage = QImage();
    m_backdropImage = QImage();
    m_logoImage = QImage();
    m_clearArtImage = QImage();
    m_cdArtImage = QImage();
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
int Concert::runtime() const
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
QList<QString*> Concert::genresPointer()
{
    QList<QString*> genres;
    for (int i=0, n=m_genres.size() ; i<n ; ++i)
        genres.append(&m_genres[i]);
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
 * @brief Holds the current concert poster
 * @return Current concert poster
 */
QImage *Concert::posterImage()
{
    return &m_posterImage;
}

/**
 * @brief Holds the current concert backdrop
 * @return Current concert backdrop
 */
QImage *Concert::backdropImage()
{
    return &m_backdropImage;
}

/**
 * @brief Holds the current concert logo
 * @return Current concert logo
 */
QImage *Concert::logoImage()
{
    return &m_logoImage;
}

/**
 * @brief Holds the current concert clear art
 * @return Current concert clear art
 */
QImage *Concert::clearArtImage()
{
    return &m_clearArtImage;
}

/**
 * @brief Holds the current concert cd art
 * @return Current concert cd art
 */
QImage *Concert::cdArtImage()
{
    return &m_cdArtImage;
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
 * @brief Holds wether concert infos were loaded from a MediaCenterInterface or ScraperInterface
 * @return Infos were loaded
 */
bool Concert::infoLoaded() const
{
    return m_infoLoaded;
}

/**
 * @brief Holds a property indicating if the poster image was changed
 * @return Concerts poster image was changed
 */
bool Concert::posterImageChanged() const
{
    return m_posterImageChanged;
}

/**
 * @brief Holds a property indicating if the backdrop image was changed
 * @return Concerts backdrop image was changed
 */
bool Concert::backdropImageChanged() const
{
    return m_backdropImageChanged;
}

/**
 * @brief Holds a property indicating if the logo image was changed
 * @return Concerts logo image was changed
 */
bool Concert::logoImageChanged() const
{
    return m_logoImageChanged;
}

/**
 * @brief Holds a property indicating if the clear art image was changed
 * @return Concerts clear art image was changed
 */
bool Concert::clearArtImageChanged() const
{
    return m_clearArtImageChanged;
}

/**
 * @brief Holds a property indicating if the cd art image was changed
 * @return Concerts cd art image was changed
 */
bool Concert::cdArtImageChanged() const
{
    return m_cdArtImageChanged;
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
StreamDetails *Concert::streamDetails()
{
    return m_streamDetails;
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
void Concert::setRuntime(int runtime)
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
 * @brief Sets the concerts genres
 * @param genres List of genres of the concert
 * @see Concert::genres
 */
void Concert::setGenres(QStringList genres)
{
    m_genres = genres;
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
    if (m_posters.size() < index)
        return;
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
    if (m_backdrops.size() < index)
        return;
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

/*** ADDER ***/

/**
 * @brief Adds a genre to the concert
 * @param genre Genre to add
 * @see Concert::genres
 */
void Concert::addGenre(QString genre)
{
    m_genres.append(genre);
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

/**
 * @brief Sets the current poster image
 * @param poster Current poster image
 * @see Concert::posters
 */
void Concert::setPosterImage(QImage poster)
{
    m_posterImage = QImage(poster);
    m_posterImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the current backdrop image
 * @param backdrop Current backdrop image
 * @see Concert::backdrops
 */
void Concert::setBackdropImage(QImage backdrop)
{
    m_backdropImage = QImage(backdrop);
    m_backdropImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the current logo image
 * @param img Current logo image
 */
void Concert::setLogoImage(QImage img)
{
    m_logoImage = QImage(img);
    m_logoImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the current clear art image
 * @param img Current clear art image
 */
void Concert::setClearArtImage(QImage img)
{
    m_clearArtImage = QImage(img);
    m_clearArtImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the current cd art image
 * @param img Current cd art image
 */
void Concert::setCdArtImage(QImage img)
{
    m_cdArtImage = QImage(img);
    m_cdArtImageChanged = true;
    setChanged(true);
}

/*** REMOVER ***/

/**
 * @brief Concert::removeGenre
 * @param genre
 * @see Concert::genres
 */
void Concert::removeGenre(QString *genre)
{
    for (int i=0, n=m_genres.size() ; i<n ; ++i) {
        if (&m_genres[i] == genre) {
            m_genres.removeAt(i);
            break;
        }
    }
    setChanged(true);
}
