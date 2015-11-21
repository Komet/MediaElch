#include "Movie.h"
#include "globals/NameFormatter.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "data/ImageCache.h"
#include "globals/Helper.h"
#include "settings/Settings.h"

/**
 * @brief Constructs a new movie object
 * @param files List of files for this movie
 * @param parent
 */
Movie::Movie(QStringList files, QObject *parent) :
    QObject(parent)
{
    m_controller = new MovieController(this);
    m_files = files;
    m_rating = 0;
    m_votes = 0;
    m_top250 = 0;
    m_runtime = 0;
    m_playcount = 0;
    m_watched = false;
    m_hasChanged = false;
    m_hasExtraFanarts = false;
    m_inSeparateFolder = false;
    m_syncNeeded = false;
    static int m_idCounter = 0;
    m_movieId = ++m_idCounter;
    m_mediaCenterId = -1;
    m_numPrimaryLangPosters = 0;
    m_streamDetailsLoaded = false;
    m_databaseId = -1;
    m_discType = DiscSingle;
    m_label = Labels::NO_LABEL;
    if (!files.isEmpty())
        setFiles(files);
}

Movie::~Movie()
{
}

void Movie::setFiles(QStringList files)
{
    m_files = files;
    if (files.size() > 0) {
        QFileInfo fi(files.at(0));
        QStringList path = fi.path().split("/", QString::SkipEmptyParts);
        if (!path.isEmpty())
            m_folderName = path.last();
    }
    if (!files.isEmpty())
        m_streamDetails = new StreamDetails(this, files);
    else
        m_streamDetails = new StreamDetails(this, QStringList());
}

MovieController *Movie::controller()
{
    return m_controller;
}

/**
 * @brief Clears all infos in the movie
 */
void Movie::clear()
{
    QList<int> infos;
    infos << MovieScraperInfos::Title
          << MovieScraperInfos::Set
          << MovieScraperInfos::Tagline
          << MovieScraperInfos::Rating
          << MovieScraperInfos::Released
          << MovieScraperInfos::Runtime
          << MovieScraperInfos::Certification
          << MovieScraperInfos::Trailer
          << MovieScraperInfos::Overview
          << MovieScraperInfos::Poster
          << MovieScraperInfos::Backdrop
          << MovieScraperInfos::Actors
          << MovieScraperInfos::Genres
          << MovieScraperInfos::Studios
          << MovieScraperInfos::Countries
          << MovieScraperInfos::Writer
          << MovieScraperInfos::Director
          << MovieScraperInfos::Tags
          << MovieScraperInfos::ExtraFanarts
          << MovieScraperInfos::Logo
          << MovieScraperInfos::CdArt
          << MovieScraperInfos::Banner
          << MovieScraperInfos::Thumb
          << MovieScraperInfos::ClearArt;
    clear(infos);
    m_nfoContent.clear();
}

/**
 * @brief Clears contents of the movie based on a list
 * @param infos List of infos which should be cleared
 */
void Movie::clear(QList<int> infos)
{
    if (infos.contains(MovieScraperInfos::Actors))
        m_actors.clear();
    if (infos.contains(MovieScraperInfos::Backdrop)) {
        m_backdrops.clear();
        m_images.insert(ImageType::MovieBackdrop, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieBackdrop, false);
        m_imagesToRemove.removeOne(ImageType::MovieBackdrop);
    }
    if (infos.contains(MovieScraperInfos::CdArt)) {
        m_discArts.clear();
        m_images.insert(ImageType::MovieCdArt, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieCdArt, false);
        m_imagesToRemove.removeOne(ImageType::MovieCdArt);
    }
    if (infos.contains(MovieScraperInfos::ClearArt)) {
        m_clearArts.clear();
        m_images.insert(ImageType::MovieClearArt, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieClearArt, false);
        m_imagesToRemove.removeOne(ImageType::MovieClearArt);
    }
    if (infos.contains(MovieScraperInfos::Logo)) {
        m_logos.clear();
        m_images.insert(ImageType::MovieLogo, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieLogo, false);
        m_imagesToRemove.removeOne(ImageType::MovieLogo);
    }
    if (infos.contains(MovieScraperInfos::Countries))
        m_countries.clear();
    if (infos.contains(MovieScraperInfos::Genres))
        m_genres.clear();
    if (infos.contains(MovieScraperInfos::Poster)){
        m_posters.clear();
        m_images.insert(ImageType::MoviePoster, QByteArray());
        m_hasImageChanged.insert(ImageType::MoviePoster, false);
        m_numPrimaryLangPosters = 0;
        m_imagesToRemove.removeOne(ImageType::MoviePoster);
    }
    if (infos.contains(MovieScraperInfos::Studios))
        m_studios.clear();
    if (infos.contains(MovieScraperInfos::Title))
        m_originalName = "";
    if (infos.contains(MovieScraperInfos::Set))
        m_set = "";
    if (infos.contains(MovieScraperInfos::Overview)) {
        m_overview = "";
        m_outline = "";
    }
    if (infos.contains(MovieScraperInfos::Rating)) {
        m_rating = 0;
        m_votes = 0;
    }
    if (infos.contains(MovieScraperInfos::Released))
        m_released = QDate(2000, 02, 30); // invalid date
    if (infos.contains(MovieScraperInfos::Tagline))
        m_tagline = "";
    if (infos.contains(MovieScraperInfos::Runtime))
        m_runtime = 0;
    if (infos.contains(MovieScraperInfos::Trailer))
        m_trailer = "";
    if (infos.contains(MovieScraperInfos::Certification))
        m_certification = "";
    if (infos.contains(MovieScraperInfos::Writer))
        m_writer = "";
    if (infos.contains(MovieScraperInfos::Director))
        m_director = "";
    if (infos.contains(MovieScraperInfos::Tags))
        m_tags.clear();

    if (infos.contains(MovieScraperInfos::Banner)) {
        m_images.insert(ImageType::MovieBanner, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieBanner, false);
        m_imagesToRemove.removeOne(ImageType::MovieBanner);
    }
    if (infos.contains(MovieScraperInfos::Thumb)) {
        m_images.insert(ImageType::MovieThumb, QByteArray());
        m_hasImageChanged.insert(ImageType::MovieThumb, false);
        m_imagesToRemove.removeOne(ImageType::MovieThumb);
    }
    if (infos.contains(MovieScraperInfos::ExtraFanarts)) {
        m_extraFanartsToRemove.clear();
        m_extraFanartImagesToAdd.clear();
        m_extraFanarts.clear();
    }
}

/**
 * @brief Clears the movie images to save memory
 */
void Movie::clearImages()
{
    m_images.clear();
    m_hasImageChanged.clear();
    m_extraFanartImagesToAdd.clear();
    foreach (Actor *actor, actorsPointer())
        actor->image = QByteArray();
}

/*** GETTER ***/

/**
 * @property Movie::name
 * @brief Holds the movies name
 * @return The movies name
 * @see Movie::setName
 */
QString Movie::name() const
{
    return m_name;
}

/**
 * @property Movie::sortTitle
 * @brief Holds the sort title
 * @return Sort title of the movie
 * @see Movie::setSortTitle
 */
QString Movie::sortTitle() const
{
    return m_sortTitle;
}

/**
 * @property Movie::originalName
 * @brief Holds the original name
 * @return Original name of the movie
 * @see Movie::setOriginalName
 */
QString Movie::originalName() const
{
    return m_originalName;
}

/**
 * @property Movie::overview
 * @brief Holds the movies plot
 * @return Plot of the movie
 * @see Movie::setOverview
 */
QString Movie::overview() const
{
    return m_overview;
}

/**
 * @brief Holds the movies rating
 * @return Rating of the movie
 * @see Movie::setRating
 */
qreal Movie::rating() const
{
    return m_rating;
}

/**
 * @brief Holds the movies votes
 * @return Votes of the movie
 * @see Movie::setVotes
 */
int Movie::votes() const
{
    return m_votes;
}

/**
 * @brief Holds the movies top 250
 * @return Position of the movie in top 250
 * @see Movie::setTop250
 */
int Movie::top250() const
{
    return m_top250;
}

/**
 * @property Movie::released
 * @brief Holds the movies release date
 * @return Release date of the movie
 * @see Movie::setReleased
 */
QDate Movie::released() const
{
    return m_released;
}

/**
 * @property Movie::tagline
 * @brief Holds the movies tagline
 * @return Tagline of the movie
 * @see Movie::setTagline
 */
QString Movie::tagline() const
{
    return m_tagline;
}

/**
 * @property Movie::outline
 * @brief Holds the movies outline
 * @return Outline of the movie
 * @see Movie::setOutline
 */
QString Movie::outline() const
{
    return m_outline;
}

/**
 * @brief Holds the movies runtime
 * @return Runtime of the movie
 * @see Movie::setRuntime
 */
int Movie::runtime() const
{
    return m_runtime;
}

/**
 * @property Movie::certification
 * @brief Holds the movies certification
 * @return Certification of the movie
 * @see Movie::setCertification
 */
QString Movie::certification() const
{
    return m_certification;
}

/**
 * @property Movie::writer
 * @brief Holds the movies writer
 * @return Writer of the movie
 * @see Movie::setWriter
 */
QString Movie::writer() const
{
    return m_writer;
}

/**
 * @property Movie::director
 * @brief Holds the movies director
 * @return Director of the movie
 * @see Movie::setDirector
 */
QString Movie::director() const
{
    return m_director;
}

/**
 * @property Movie::genres
 * @brief Holds a list of the movies genres
 * @return List of genres of the movie
 * @see Movie::setGenres
 * @see Movie::genresPointer
 * @see Movie::addGenre
 * @see Movie::removeGenre
 */
QStringList Movie::genres() const
{
    return m_genres;
}

/**
 * @brief Returns a list of pointers to QStrings
 * @return List of pointers to the movies genres
 */
QList<QString*> Movie::genresPointer()
{
    QList<QString*> genres;
    for (int i=0, n=m_genres.size() ; i<n ; ++i)
        genres.append(&m_genres[i]);
    return genres;
}

/**
 * @property Movie::countries
 * @brief Holds the movies countries
 * @return List of production countries of the movie
 * @see Movie::setCountries
 * @see Movie::countriesPointer
 * @see Movie::addCountry
 * @see Movie::removeCountry
 */
QStringList Movie::countries() const
{
    return m_countries;
}

/**
 * @brief Returns a list of pointers to QStrings
 * @return List of pointers to the movies production countries
 */
QList<QString*> Movie::countriesPointer()
{
    QList<QString*> countries;
    for (int i=0, n=m_countries.size() ; i<n ; ++i)
        countries.append(&m_countries[i]);
    return countries;
}

/**
 * @property Movie::studios
 * @brief Holds the movies studios
 * @return List of studios of the movies
 * @see Movie::setStudios
 * @see Movie::studiosPointer
 * @see Movie::addStudio
 * @see Movie::removeStudio
 */
QStringList Movie::studios() const
{
    return m_studios;
}

/**
 * @brief Returns a list of pointers of QStrings
 * @return List of pointers to the movies studios
 */
QList<QString*> Movie::studiosPointer()
{
    QList<QString*> studios;
    for (int i=0, n=m_studios.size() ; i<n ; ++i)
        studios.append(&m_studios[i]);
    return studios;
}

/**
 * @property Movie::trailer
 * @brief Holds the movies trailer
 * @return Trailer of the movie
 * @see Movie::setTrailer
 */
QUrl Movie::trailer() const
{
    return m_trailer;
}

/**
 * @property Movie::actors
 * @brief Holds the movies actors
 * @return List of actors of the movie
 * @see Movie::setActors
 * @see Movie::actorsPointer
 * @see Movie::addActor
 * @see Movie::removeActor
 */
QList<Actor> Movie::actors() const
{
    return m_actors;
}

/**
 * @brief Returns a list of pointers of Actor
 * @return List of pointers to movies actors
 */
QList<Actor*> Movie::actorsPointer()
{
    QList<Actor*> actors;
    for (int i=0, n=m_actors.size() ; i<n ; i++)
        actors.append(&(m_actors[i]));
    return actors;
}

/**
 * @brief Holds the files of the movie
 * @return List of files
 */
QStringList Movie::files() const
{
    return m_files;
}

/**
 * @property Movie::playcount
 * @brief Holds the playcount
 * @return Playcount of the movie
 * @see Movie::setPlayCount
 */
int Movie::playcount() const
{
    return m_playcount;
}

/**
 * @property Movie::lastPlayed
 * @brief Holds the date when the movie was last played
 *        If the movie was never played an invalid date will be returned
 * @return Date of last playtime
 * @see Movie::setLastPlayed
 */
QDateTime Movie::lastPlayed() const
{
    return m_lastPlayed;
}

/**
 * @property Movie::id
 * @brief Holds the movies id
 * @return Id of the movie
 * @see Movie::setId
 */
QString Movie::id() const
{
    return m_id;
}

/**
 * @property Movie::tmdbId
 * @brief Holds the movies tmdb id
 * @return Tmdb id of the movie
 * @see Movie::setTmdbId
 */
QString Movie::tmdbId() const
{
    return m_tmdbId;
}

QString Movie::mediaPassionId() const
{
    return m_mediaPassionId;
}

/**
 * @property Movie::set
 * @brief Holds the set of the movie
 * @return Set of the movie
 * @see Movie::setSet
 */
QString Movie::set() const
{
    return m_set;
}

/**
 * @property Movie::posters
 * @brief Holds a list of posters of the movie
 * @return List of posters
 * @see Movie::setPosters
 * @see Movie::setPoster
 * @see Movie::addPoster
 */
QList<Poster> Movie::posters() const
{
    return m_posters;
}

/**
 * @property Movie::backdrops
 * @brief Holds a list of backdrops of the movie
 * @return List of backdrops
 * @see Movie::setBackdrops
 * @see Movie::setBackdrop
 * @see Movie::addBackdrop
 */
QList<Poster> Movie::backdrops() const
{
    return m_backdrops;
}

QList<Poster> Movie::discArts() const
{
    return m_discArts;
}

QList<Poster> Movie::clearArts() const
{
    return m_clearArts;
}

QList<Poster> Movie::logos() const
{
    return m_logos;
}

/**
 * @brief Returns the parent folder of the movie
 * @return Parent folder of the movie
 */
QString Movie::folderName() const
{
    return m_folderName;
}

/**
 * @property Movie::watched
 * @brief Holds the movies watched status
 * @return Watched status of the movie
 * @see Movie::hasWatched
 */
bool Movie::watched() const
{
    return m_watched;
}

/**
 * @property Movie::hasChanged
 * @brief Holds a property if the movies infos were changed by a setter or a ScraperInterface
 * @return True if some of the movies infos were changed
 * @see Movie::setChanged
 */
bool Movie::hasChanged() const
{
    return m_hasChanged;
}

/**
 * @property Movie::streamDetailsLoaded
 * @brief Holds if the stream details were loaded
 * @return True if the stream details were loaded
 * @see Movie::setStreamDetailsLoaded
 */
bool Movie::streamDetailsLoaded() const
{
    return m_streamDetailsLoaded;
}

/**
 * @brief Holds a unique MediaElch movie id
 * @return MediaElchs id of the movie
 */
int Movie::movieId() const
{
    return m_movieId;
}

/**
 * @brief Holds if the movies files are stored in a separate folder
 * @return Movies files are stored in a separate folder
 */
bool Movie::inSeparateFolder() const
{
    return m_inSeparateFolder;
}

/**
 * @brief Movie::mediaCenterId
 * @return Id in a MediaCenterInterface
 */
int Movie::mediaCenterId() const
{
    return m_mediaCenterId;
}

/**
 * @brief Returns how many of the posters were scraped in the primary language
 * @return Number of primary language posters
 */
int Movie::numPrimaryLangPosters() const
{
    return m_numPrimaryLangPosters;
}

/**
 * @brief The stream details object of this movie
 * @return StreamDetails Object
 */
StreamDetails *Movie::streamDetails()
{
    return m_streamDetails;
}

/**
 * @brief The last modification date of the file
 * @return Last mod date
 */
QDateTime Movie::fileLastModified() const
{
    return m_fileLastModified;
}

/**
 * @brief Movie::nfoContent
 * @return
 */
QString Movie::nfoContent() const
{
    return m_nfoContent;
}

/**
 * @brief Movie::databaseId
 * @return
 */
int Movie::databaseId() const
{
    return m_databaseId;
}

bool Movie::syncNeeded() const
{
    return m_syncNeeded;
}

QStringList Movie::tags() const
{
    return m_tags;
}

/*** SETTER ***/

/**
 * @brief Sets the movies name
 * @param name Name of the movie
 * @see Movie::name
 */
void Movie::setName(QString name)
{
    m_name = name;
    setChanged(true);
}

/**
 * @brief Sets the movies sort title
 * @param sortTitle Sort title of the movie
 * @see Movie::sortTitle
 */
void Movie::setSortTitle(QString sortTitle)
{
    m_sortTitle = sortTitle;
    setChanged(true);
}

/**
 * @brief Sets the movies original name
 * @param originalName Original name of the movie
 * @see Movie::originalName
 */
void Movie::setOriginalName(QString originalName)
{
    m_originalName = originalName;
    setChanged(true);
}

/**
 * @brief Sets the movies plot
 * @param overview Plot of the movie
 * @see Movie::overview
 */
void Movie::setOverview(QString overview)
{
    m_overview = overview;
    setChanged(true);
}

/**
 * @brief Sets the movies rating
 * @param rating Rating of the movie
 * @see Movie::rating
 */
void Movie::setRating(qreal rating)
{
    m_rating = rating;
    setChanged(true);
}

/**
 * @brief Sets the movies votes
 * @param votes Votes of the movie
 * @see Movie::votes
 */
void Movie::setVotes(int votes)
{
    m_votes = votes;
    setChanged(true);
}

/**
 * @brief Sets the movies top 250 place
 * @param top250 Top 250 position of the movie
 * @see Movie::top250
 */
void Movie::setTop250(int top250)
{
    m_top250 = top250;
    setChanged(true);
}

/**
 * @brief Sets the movies release date
 * @param released Release date of the movie
 * @see Movie::released
 */
void Movie::setReleased(QDate released)
{
    m_released = released;
    setChanged(true);
}

/**
 * @brief Sets the movies tagline
 * @param tagline Tagline of the movie
 * @see Movie::tagline
 */
void Movie::setTagline(QString tagline)
{
    m_tagline = tagline;
    setChanged(true);
}

/**
 * @brief Sets the movies outline
 * @param outline Outline of the movie
 * @see Movie::outline
 */
void Movie::setOutline(QString outline)
{
    m_outline = outline;
    setChanged(true);
}

/**
 * @brief Sets the movies runtime
 * @param runtime Runtime in minutes
 * @see Movie::runtime
 */
void Movie::setRuntime(int runtime)
{
    m_runtime = runtime;
    setChanged(true);
}

/**
 * @brief Sets the movies certification
 * @param certification Certification of the movie
 * @see Movie::certification
 */
void Movie::setCertification(QString certification)
{
    m_certification = certification;
    setChanged(true);
}

/**
 * @brief Sets the movies writer
 * @param writer Writer of the movie
 * @see Movie::writer
 */
void Movie::setWriter(QString writer)
{
    m_writer = writer;
    setChanged(true);
}

/**
 * @brief Sets the movies director
 * @param director Director of the movie
 * @see Movie::director
 */
void Movie::setDirector(QString director)
{
    m_director = director;
    setChanged(true);
}

/**
 * @brief Sets the movies trailer
 * @param trailer URL of the movies trailer
 * @see Movie::trailer
 */
void Movie::setTrailer(QUrl trailer)
{
    m_trailer = trailer;
    setChanged(true);
}

/**
 * @brief Sets the movies actors
 * @param actors List of actors
 * @see Movie::actors
 */
void Movie::setActors(QList<Actor> actors)
{
    m_actors = actors;
    setChanged(true);
}

/**
 * @brief Sets the movies playcount
 * @param playcount Playcount of the movie
 * @see Movie::playcount
 */
void Movie::setPlayCount(int playcount)
{
    m_playcount = playcount;
    setChanged(true);
}

/**
 * @brief Sets the movies last playtime. If the movie has never played, set an invalid date.
 * @param lastPlayed Last playtime of the movie
 * @see Movie::lastPlayed
 */
void Movie::setLastPlayed(QDateTime lastPlayed)
{
    m_lastPlayed = lastPlayed;
    setChanged(true);
}

/**
 * @brief Sets the id of the movie
 * @param id Id of the movie
 * @see Movie::id
 */
void Movie::setId(QString id)
{
    m_id = id;
    setChanged(true);
}

/**
 * @brief Sets the tmdb id of the movie
 * @param id Tmdb id of the movie
 * @see Movie::tmdbId
 */
void Movie::setTmdbId(QString id)
{
    m_tmdbId = id;
    setChanged(true);
}

void Movie::setMediaPassionId(QString id)
{
    m_mediaPassionId = id;
    setChanged(true);
}

/**
 * @brief Sets the movies set
 * @param set Setname of the movie
 * @see Movie::set
 */
void Movie::setSet(QString set)
{
    m_set = set;
    setChanged(true);
}

/**
 * @brief Sets the movies watched status
 * @param watched Watched status of the movie
 * @see Movie::watched
 */
void Movie::setWatched(bool watched)
{
    m_watched = watched;
    setChanged(true);
}

/**
 * @brief Sets if some of the movies info has changed. Emits the sigChanged signal
 * @param changed Infos have changed
 * @see Movie::hasChanged
 */
void Movie::setChanged(bool changed)
{
    m_hasChanged = changed;
    emit sigChanged(this);
}

/**
 * @brief Sets if the stream details were loaded
 * @param loaded
 * @see Movie::streamDetailsLoaded
 */
void Movie::setStreamDetailsLoaded(bool loaded)
{
    m_streamDetailsLoaded = loaded;
}

/**
 * @brief Sets if the movies files are stored in a separate folder
 * @param inSepFolder Files of the movie are in one separate folder
 */
void Movie::setInSeparateFolder(bool inSepFolder)
{
    m_inSeparateFolder = inSepFolder;
}

/**
 * @brief Sets the media center id of the movie
 * @param mediaCenterId Id of the movie
 */
void Movie::setMediaCenterId(int mediaCenterId)
{
    m_mediaCenterId = mediaCenterId;
}

/**
 * @brief Sets the number of primary language posters
 * @param numberPrimaryLangPosters Number of primary language posters
 */
void Movie::setNumPrimaryLangPosters(int numberPrimaryLangPosters)
{
    m_numPrimaryLangPosters = numberPrimaryLangPosters;
}

/**
 * @brief Sets the last modification date
 * @param modified Last mod date
 */
void Movie::setFileLastModified(QDateTime modified)
{
    m_fileLastModified = modified;
}

/**
 * @brief Movie::setNfoContent
 * @param content
 */
void Movie::setNfoContent(QString content)
{
    m_nfoContent = content;
}

/**
 * @brief Movie::setDatabaseId
 * @param id
 */
void Movie::setDatabaseId(int id)
{
    m_databaseId = id;
}

void Movie::setSyncNeeded(bool syncNeeded)
{
    m_syncNeeded = syncNeeded;
}

/*** ADDER ***/

/**
 * @brief Adds an actor to the movie
 * @param actor Actor to add
 * @see Movie::actors
 */
void Movie::addActor(Actor actor)
{
    m_actors.append(actor);
    setChanged(true);
}

/**
 * @brief Adds a country to the movie
 * @param country Country to add
 * @see Movie::countries
 */
void Movie::addCountry(QString country)
{
    if (country.isEmpty())
        return;
    m_countries.append(country);
    setChanged(true);
}

/**
 * @brief Adds a genre to the movie
 * @param genre Genre to add
 * @see Movie::genres
 */
void Movie::addGenre(QString genre)
{
    if (genre.isEmpty())
        return;
    m_genres.append(genre);
    setChanged(true);
}

/**
 * @brief Adds a studio to the movie
 * @param studio Studio to add
 * @see Movie::studios
 */
void Movie::addStudio(QString studio)
{
    if (studio.isEmpty())
        return;
    m_studios.append(studio);
    setChanged(true);
}

void Movie::addTag(QString tag)
{
    if (m_tags.contains(tag))
        return;
    m_tags.append(tag);
    setChanged(true);
}

/**
 * @brief Adds a poster to the movie
 * @param poster Poster to add
 * @param primaryLang Poster is in primary language
 * @see Movie::posters
 */
void Movie::addPoster(Poster poster, bool primaryLang)
{
    if(primaryLang){
        m_posters.insert(m_numPrimaryLangPosters,poster);
        m_numPrimaryLangPosters++;
    } else{
        m_posters.append(poster);
    }
    setChanged(true);
}

/**
 * @brief Adds a backdrop to the movie
 * @param backdrop Backdrop to add
 * @see Movie::backdrops
 */
void Movie::addBackdrop(Poster backdrop)
{
    m_backdrops.append(backdrop);
    setChanged(true);
}

void Movie::addDiscArt(Poster discArt)
{
    m_discArts.append(discArt);
    setChanged(true);
}

void Movie::addClearArt(Poster clearArt)
{
    m_clearArts.append(clearArt);
    setChanged(true);
}

void Movie::addLogo(Poster logo)
{
    m_logos.append(logo);
    setChanged(true);
}

/*** REMOVER ***/

/**
 * @brief Removes an actor from the movie
 * @param actor Pointer to the actor to remove
 * @see Movie::actors
 */
void Movie::removeActor(Actor *actor)
{
    for (int i=0, n=m_actors.size() ; i<n ; ++i) {
        if (&m_actors[i] == actor) {
            m_actors.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

/**
 * @brief Removes a production country from the movie
 * @param country Pointer to the country to remove
 * @see Movie::countries
 */
void Movie::removeCountry(QString *country)
{
    for (int i=0, n=m_countries.size() ; i<n ; ++i) {
        if (&m_countries[i] == country) {
            m_countries.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

/**
 * @brief Removes a production country from the movie
 * @param country Country to remove
 * @see Movie::countries
 */
void Movie::removeCountry(QString country)
{
    m_countries.removeAll(country);
    setChanged(true);
}

/**
 * @brief Movie::removeGenre
 * @param genre
 * @see Movie::genres
 */
void Movie::removeGenre(QString *genre)
{
    for (int i=0, n=m_genres.size() ; i<n ; ++i) {
        if (&m_genres[i] == genre) {
            m_genres.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

/**
 * @brief Movie::removeGenre
 * @param genre
 * @see Movie::genres
 */
void Movie::removeGenre(QString genre)
{
    m_genres.removeAll(genre);
    setChanged(true);
}

/**
 * @brief Removes a studio from the movie
 * @param studio Pointer to the studio to remove
 * @see Movie::studios
 */
void Movie::removeStudio(QString *studio)
{
    for (int i=0, n=m_studios.size() ; i<n ; ++i) {
        if (&m_studios[i] == studio) {
            m_studios.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

/**
 * @brief Removes a studio from the movie
 * @param studio Studio to remove
 * @see Movie::studios
 */
void Movie::removeStudio(QString studio)
{
    m_studios.removeAll(studio);
    setChanged(true);
}

void Movie::removeTag(QString tag)
{
    m_tags.removeAll(tag);
    setChanged(true);
}

bool Movie::hasLocalTrailer() const
{
    if (files().count() == 0)
        return false;
    QFileInfo fi(files().first());
    QString trailerFilter = QString("%1-trailer*").arg(fi.completeBaseName());
    QDir dir(fi.canonicalPath());
    return !dir.entryList(QStringList() << trailerFilter).isEmpty();
}

QString Movie::localTrailerFileName() const
{
    if (files().count() == 0)
        return QString();
    QFileInfo fi(files().first());
    QString trailerFilter = QString("%1-trailer*").arg(fi.completeBaseName());
    QDir dir(fi.canonicalPath());

    QStringList contents = dir.entryList(QStringList() << trailerFilter);
    if (contents.isEmpty())
        return QString();

    return dir.absolutePath() + "/" + contents.first();
}

void Movie::addExtraFanart(QByteArray fanart)
{
    m_extraFanartImagesToAdd.append(fanart);
    setChanged(true);
}

void Movie::removeExtraFanart(QByteArray fanart)
{
    m_extraFanartImagesToAdd.removeOne(fanart);
    setChanged(true);
}

void Movie::removeExtraFanart(QString file)
{
    m_extraFanarts.removeOne(file);
    m_extraFanartsToRemove.append(file);
    setChanged(true);
}

QList<ExtraFanart> Movie::extraFanarts(MediaCenterInterface *mediaCenterInterface)
{
    if (m_extraFanarts.isEmpty())
        m_extraFanarts = mediaCenterInterface->extraFanartNames(this);
    foreach (const QString &file, m_extraFanartsToRemove)
        m_extraFanarts.removeOne(file);
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

QStringList Movie::extraFanartsToRemove()
{
    return m_extraFanartsToRemove;
}

QList<QByteArray> Movie::extraFanartImagesToAdd()
{
    return m_extraFanartImagesToAdd;
}

void Movie::clearExtraFanartData()
{
    m_extraFanartImagesToAdd.clear();
    m_extraFanartsToRemove.clear();
    m_extraFanarts.clear();
}

void Movie::setDateAdded(QDateTime date)
{
    m_dateAdded = date;
}

QDateTime Movie::dateAdded() const
{
    return m_dateAdded;
}

void Movie::setDiscType(DiscType type)
{
    m_discType = type;
}

DiscType Movie::discType()
{
    return m_discType;
}

void Movie::setHasExtraFanarts(bool has)
{
    m_hasExtraFanarts = has;
}

bool Movie::hasExtraFanarts() const
{
    return m_hasExtraFanarts;
}

QList<int> Movie::imagesToRemove() const
{
    return m_imagesToRemove;
}

void Movie::removeImage(int type)
{
    if (!m_images.value(type, QByteArray()).isNull()) {
        m_images.remove(type);
        m_hasImageChanged.insert(type, false);
    } else if (!m_imagesToRemove.contains(type)) {
        m_imagesToRemove.append(type);
    }
    setChanged(true);
}

QByteArray Movie::image(int imageType)
{
    return m_images.value(imageType, QByteArray());
}

bool Movie::imageHasChanged(int imageType)
{
    return m_hasImageChanged.value(imageType, false);
}

bool Movie::hasImage(int imageType)
{
    return m_hasImage.value(imageType, false);
}

void Movie::setHasImage(int imageType, bool has)
{
    m_hasImage.insert(imageType, has);
}

void Movie::setImage(int imageType, QByteArray image)
{
    m_images.insert(imageType, image);
    m_hasImageChanged.insert(imageType, true);
    setChanged(true);
}

bool Movie::lessThan(Movie *a, Movie *b)
{
    return (QString::localeAwareCompare(Helper::instance()->appendArticle(a->name()), Helper::instance()->appendArticle(b->name())) < 0);
}

QList<int> Movie::imageTypes()
{
    return QList<int>() << ImageType::MoviePoster << ImageType::MovieBanner
                        << ImageType::MovieCdArt << ImageType::MovieClearArt
                        << ImageType::MovieLogo << ImageType::MovieThumb
                        << ImageType::MovieBackdrop;
}

QList<Subtitle *> Movie::subtitles() const
{
    return m_subtitles;
}

void Movie::setSubtitles(const QList<Subtitle *> &subtitles)
{
    m_subtitles = subtitles;
}

void Movie::addSubtitle(Subtitle *subtitle, bool fromLoad)
{
    m_subtitles.append(subtitle);
    connect(subtitle, &Subtitle::sigChanged, this, &Movie::onSubtitleChanged);
    if (!fromLoad)
        setChanged(true);
}

void Movie::onSubtitleChanged()
{
    setChanged(true);
}

void Movie::setLabel(int label)
{
    m_label = label;
}

int Movie::label()
{
    return m_label;
}

/*** DEBUG ***/

QDebug operator<<(QDebug dbg, const Movie &movie)
{
    QString nl = "\n";
    QString out;
    out.append("Movie").append(nl);
    out.append(QString("  Files:         ").append(nl));
    foreach (const QString &file, movie.files())
        out.append(QString("    %1").arg(file).append(nl));
    out.append(QString("  Name:          ").append(movie.name()).append(nl));
    out.append(QString("  Original-Name: ").append(movie.originalName()).append(nl));
    out.append(QString("  Rating:        %1").arg(movie.rating()).append(nl));
    out.append(QString("  Released:      ").append(movie.released().toString("yyyy-MM-dd")).append(nl));
    out.append(QString("  Tagline:       ").append(movie.tagline()).append(nl));
    out.append(QString("  Runtime:       %1").arg(movie.runtime()).append(nl));
    out.append(QString("  Certification: ").append(movie.certification()).append(nl));
    out.append(QString("  Playcount:     %1%2").arg(movie.playcount()).arg(nl));
    out.append(QString("  Lastplayed:    ").append(movie.lastPlayed().toString("yyyy-MM-dd HH:mm:ss")).append(nl));
    out.append(QString("  ID:            ").append(movie.id()).append(nl));
    out.append(QString("  Set:           ").append(movie.set()).append(nl));
    out.append(QString("  Overview:      ").append(movie.overview())).append(nl);
    foreach (const QString &studio, movie.studios())
        out.append(QString("  Studio:         ").append(studio)).append(nl);
    foreach (const QString &genre, movie.genres())
        out.append(QString("  Genre:         ").append(genre)).append(nl);
    foreach (const QString &country, movie.countries())
        out.append(QString("  Country:       ").append(country)).append(nl);
    foreach (const Actor &actor, movie.actors()) {
        out.append(QString("  Actor:         ").append(nl));
        out.append(QString("    Name:  ").append(actor.name)).append(nl);
        out.append(QString("    Role:  ").append(actor.role)).append(nl);
        out.append(QString("    Thumb: ").append(actor.thumb)).append(nl);
    }
    foreach (const Poster &poster, movie.posters()) {
        out.append(QString("  Poster:       ")).append(nl);
        out.append(QString("    ID:       ").append(poster.id)).append(nl);
        out.append(QString("    Original: ").append(poster.originalUrl.toString())).append(nl);
        out.append(QString("    Thumb:    ").append(poster.thumbUrl.toString())).append(nl);
    }
    foreach (const Poster &backdrop, movie.backdrops()) {
        out.append(QString("  Backdrop:       ")).append(nl);
        out.append(QString("    ID:       ").append(backdrop.id)).append(nl);
        out.append(QString("    Original: ").append(backdrop.originalUrl.toString())).append(nl);
        out.append(QString("    Thumb:    ").append(backdrop.thumbUrl.toString())).append(nl);
    }
    dbg.nospace() << out;
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const Movie *movie)
{
    dbg.nospace() << *movie;
    return dbg.space();
}
