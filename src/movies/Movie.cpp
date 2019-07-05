#include "Movie.h"
#include "globals/NameFormatter.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "data/ImageCache.h"
#include "globals/Helper.h"
#include "media_centers/MediaCenterInterface.h"
#include "settings/Settings.h"

using namespace std::chrono_literals;

/**
 * @brief Constructs a new movie object
 * @param files List of files for this movie
 * @param parent
 */
Movie::Movie(QStringList files, QObject* parent) :
    QObject(parent),
    m_controller{new MovieController(this)},
    m_movieImages(*this),
    m_runtime{0min},
    m_playcount{0},
    m_databaseId{-1},
    m_mediaCenterId{-1},
    m_watched{false},
    m_hasChanged{false},
    m_inSeparateFolder{false},
    m_syncNeeded{false},
    m_streamDetailsLoaded{false},
    m_hasDuplicates{false},
    m_streamDetails{nullptr},
    m_discType{DiscType::Single},
    m_label{ColorLabel::NoLabel}
{
    static int m_idCounter = 0;
    m_movieId = ++m_idCounter;
    if (!files.isEmpty()) {
        setFiles(files);
    }
}

void Movie::setFiles(QStringList files)
{
    if (!files.isEmpty()) {
        QFileInfo fi(files.at(0));
        QStringList path = fi.path().split("/", QString::SkipEmptyParts);
        if (!path.isEmpty()) {
            m_folderName = path.last();
        }
    }
    m_files = files;
    if (m_streamDetails != nullptr) {
        m_streamDetails->deleteLater();
    }
    m_streamDetails = new StreamDetails(this, files);
}

MovieController* Movie::controller() const
{
    return m_controller;
}

/**
 * @brief Clears all infos in the movie
 */
void Movie::clear()
{
    QVector<MovieScraperInfos> infos;
    infos << MovieScraperInfos::Title         //
          << MovieScraperInfos::Set           //
          << MovieScraperInfos::Tagline       //
          << MovieScraperInfos::Rating        //
          << MovieScraperInfos::Released      //
          << MovieScraperInfos::Runtime       //
          << MovieScraperInfos::Certification //
          << MovieScraperInfos::Trailer       //
          << MovieScraperInfos::Overview      //
          << MovieScraperInfos::Poster        //
          << MovieScraperInfos::Backdrop      //
          << MovieScraperInfos::Actors        //
          << MovieScraperInfos::Genres        //
          << MovieScraperInfos::Studios       //
          << MovieScraperInfos::Countries     //
          << MovieScraperInfos::Writer        //
          << MovieScraperInfos::Director      //
          << MovieScraperInfos::Tags          //
          << MovieScraperInfos::ExtraFanarts  //
          << MovieScraperInfos::Logo          //
          << MovieScraperInfos::CdArt         //
          << MovieScraperInfos::Banner        //
          << MovieScraperInfos::Thumb         //
          << MovieScraperInfos::ClearArt;
    clear(infos);
    m_nfoContent.clear();
}

/**
 * @brief Clears contents of the movie based on a list
 * @param infos List of infos which should be cleared
 */
void Movie::clear(QVector<MovieScraperInfos> infos)
{
    if (infos.contains(MovieScraperInfos::Actors)) {
        m_crew.actors().clear();
    }
    m_movieImages.clear(infos);
    if (infos.contains(MovieScraperInfos::Countries)) {
        m_countries.clear();
    }
    if (infos.contains(MovieScraperInfos::Genres)) {
        m_genres.clear();
    }
    if (infos.contains(MovieScraperInfos::Studios)) {
        m_studios.clear();
    }
    if (infos.contains(MovieScraperInfos::Title)) {
        m_originalName = "";
    }
    if (infos.contains(MovieScraperInfos::Set)) {
        m_set = MovieSet{};
    }
    if (infos.contains(MovieScraperInfos::Overview)) {
        m_overview = "";
        m_outline = "";
    }
    if (infos.contains(MovieScraperInfos::Rating)) {
        m_ratings.clear();
    }
    if (infos.contains(MovieScraperInfos::Released)) {
        m_released = QDate(2000, 02, 30); // invalid date
    }
    if (infos.contains(MovieScraperInfos::Tagline)) {
        m_tagline = "";
    }
    if (infos.contains(MovieScraperInfos::Runtime)) {
        m_runtime = 0min;
    }
    if (infos.contains(MovieScraperInfos::Trailer)) {
        m_trailer = "";
    }
    if (infos.contains(MovieScraperInfos::Certification)) {
        m_certification = Certification::NoCertification;
    }
    if (infos.contains(MovieScraperInfos::Writer)) {
        m_crew.setWriter("");
    }
    if (infos.contains(MovieScraperInfos::Director)) {
        m_crew.setDirector("");
    }
    if (infos.contains(MovieScraperInfos::Tags)) {
        m_tags.clear();
    }
}

/// @brief Clears the movie images to save memory
void Movie::clearImages()
{
    m_movieImages.clearImages();
    for (Actor& actor : m_crew.actors()) {
        actor.image = QByteArray();
    }
}

/*** GETTER ***/

QString Movie::name() const
{
    return m_name;
}

QString Movie::sortTitle() const
{
    return m_sortTitle;
}

QString Movie::originalName() const
{
    return m_originalName;
}

MovieImages& Movie::images()
{
    return m_movieImages;
}

const MovieImages& Movie::constImages() const
{
    return m_movieImages;
}

QString Movie::overview() const
{
    return m_overview;
}

QVector<Rating>& Movie::ratings()
{
    return m_ratings;
}


const QVector<Rating>& Movie::ratings() const
{
    return m_ratings;
}

int Movie::top250() const
{
    return m_imdbTop250;
}

QDate Movie::released() const
{
    return m_released;
}

QString Movie::tagline() const
{
    return m_tagline;
}

QString Movie::outline() const
{
    return m_outline;
}

std::chrono::minutes Movie::runtime() const
{
    return m_runtime;
}

Certification Movie::certification() const
{
    return m_certification;
}

QString Movie::writer() const
{
    return m_crew.writer();
}

QString Movie::director() const
{
    return m_crew.director();
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
QVector<QString*> Movie::genresPointer()
{
    QVector<QString*> genres;
    for (int i = 0, n = m_genres.size(); i < n; ++i) {
        genres.append(&m_genres[i]);
    }
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
QVector<QString*> Movie::countriesPointer()
{
    QVector<QString*> countries;
    for (int i = 0, n = m_countries.size(); i < n; ++i) {
        countries.append(&m_countries[i]);
    }
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
QVector<QString*> Movie::studiosPointer()
{
    QVector<QString*> studios;
    for (int i = 0, n = m_studios.size(); i < n; ++i) {
        studios.append(&m_studios[i]);
    }
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

const QVector<Actor>& Movie::actors() const
{
    return m_crew.actors();
}

QVector<Actor>& Movie::actors()
{
    return m_crew.actors();
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
ImdbId Movie::imdbId() const
{
    return m_imdbId;
}

/**
 * @property Movie::tmdbId
 * @brief Holds the movies tmdb id
 * @return Tmdb id of the movie
 * @see Movie::setTmdbId
 */
TmdbId Movie::tmdbId() const
{
    return m_tmdbId;
}

/**
 * @property Movie::set
 * @brief Holds the set of the movie
 * @return Set of the movie
 * @see Movie::setSet
 */
MovieSet Movie::set() const
{
    return m_set;
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
 * @brief The stream details object of this movie
 * @return StreamDetails Object
 */
StreamDetails* Movie::streamDetails()
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
 * @brief Sets the movies top 250 place
 * @param top250 Top 250 position of the movie
 * @see Movie::top250
 */
void Movie::setTop250(int top250)
{
    m_imdbTop250 = top250;
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
void Movie::setRuntime(std::chrono::minutes runtime)
{
    m_runtime = runtime;
    setChanged(true);
}

/**
 * @brief Sets the movies certification
 * @param certification Certification of the movie
 * @see Movie::certification
 */
void Movie::setCertification(Certification certification)
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
    m_crew.setWriter(writer);
    setChanged(true);
}

/**
 * @brief Sets the movies director
 * @param director Director of the movie
 * @see Movie::director
 */
void Movie::setDirector(QString director)
{
    m_crew.setDirector(director);
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
void Movie::setActors(QVector<Actor> actors)
{
    m_crew.setActors(actors);
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
void Movie::setId(ImdbId id)
{
    m_imdbId = std::move(id);
    setChanged(true);
}

/**
 * @brief Sets the tmdb id of the movie
 * @param id Tmdb id of the movie
 * @see Movie::tmdbId
 */
void Movie::setTmdbId(TmdbId tmdbId)
{
    m_tmdbId = std::move(tmdbId);
    setChanged(true);
}

/**
 * @brief Sets the movies set
 * @param set Setname of the movie
 * @see Movie::set
 */
void Movie::setSet(MovieSet set)
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
    m_crew.addActor(actor);
    setChanged(true);
}

/**
 * @brief Adds a country to the movie
 * @param country Country to add
 * @see Movie::countries
 */
void Movie::addCountry(QString country)
{
    if (country.isEmpty()) {
        return;
    }
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
    if (genre.isEmpty()) {
        return;
    }
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
    if (studio.isEmpty()) {
        return;
    }
    m_studios.append(studio);
    setChanged(true);
}

void Movie::addTag(QString tag)
{
    if (m_tags.contains(tag)) {
        return;
    }
    m_tags.append(tag);
    setChanged(true);
}

/*** REMOVER ***/

/**
 * @brief Removes an actor from the movie
 * @param actor Pointer to the actor to remove
 * @see Movie::actors
 */
void Movie::removeActor(Actor* actor)
{
    m_crew.removeActor(actor);
    setChanged(true);
}

/**
 * @brief Removes a production country from the movie
 * @param country Pointer to the country to remove
 * @see Movie::countries
 */
void Movie::removeCountry(QString* country)
{
    for (int i = 0, n = m_countries.size(); i < n; ++i) {
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
void Movie::removeGenre(QString* genre)
{
    for (int i = 0, n = m_genres.size(); i < n; ++i) {
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
void Movie::removeStudio(QString* studio)
{
    for (int i = 0, n = m_studios.size(); i < n; ++i) {
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
    if (files().isEmpty()) {
        return false;
    }
    QFileInfo fi(files().first());
    QString trailerFilter = QStringLiteral("%1*-trailer*").arg(fi.completeBaseName());
    QDir dir(fi.canonicalPath());
    return !dir.entryList({trailerFilter}).isEmpty();
}

QString Movie::localTrailerFileName() const
{
    if (files().isEmpty()) {
        return QString();
    }
    QFileInfo fi(files().first());
    QString trailerFilter = QStringLiteral("%1*-trailer*").arg(fi.completeBaseName());
    QDir dir(fi.canonicalPath());

    QStringList contents = dir.entryList({trailerFilter});
    if (contents.isEmpty()) {
        return QString();
    }

    return dir.absolutePath() + "/" + contents.first();
}

void Movie::setDateAdded(QDateTime date)
{
    m_dateAdded = date;
}

QDateTime Movie::dateAdded() const
{
    return m_dateAdded;
}

bool Movie::hasValidImdbId() const
{
    return m_imdbId.isValid();
}

bool Movie::hasImage(ImageType imageType) const
{
    return m_movieImages.hasImage(imageType);
}

void Movie::setDiscType(DiscType type)
{
    m_discType = type;
}

DiscType Movie::discType() const
{
    return m_discType;
}

bool Movie::lessThan(Movie* a, Movie* b)
{
    return (QString::localeAwareCompare(Helper::appendArticle(a->name()), Helper::appendArticle(b->name())) < 0);
}

QVector<ImageType> Movie::imageTypes()
{
    return {ImageType::MoviePoster,
        ImageType::MovieBanner,
        ImageType::MovieCdArt,
        ImageType::MovieClearArt,
        ImageType::MovieLogo,
        ImageType::MovieThumb,
        ImageType::MovieBackdrop};
}

QVector<Subtitle*> Movie::subtitles() const
{
    return m_subtitles;
}

void Movie::setSubtitles(const QVector<Subtitle*>& subtitles)
{
    m_subtitles = subtitles;
}

void Movie::addSubtitle(Subtitle* subtitle, bool fromLoad)
{
    m_subtitles.append(subtitle);
    connect(subtitle, &Subtitle::sigChanged, this, &Movie::onSubtitleChanged);
    if (!fromLoad) {
        setChanged(true);
    }
}

void Movie::onSubtitleChanged()
{
    setChanged(true);
}

bool Movie::hasDuplicates() const
{
    return m_hasDuplicates;
}

void Movie::setHasDuplicates(bool hasDuplicates)
{
    if (m_hasDuplicates == hasDuplicates) {
        return;
    }
    m_hasDuplicates = hasDuplicates;
    emit sigChanged(this);
}

void Movie::setLabel(ColorLabel label)
{
    m_label = label;
}

ColorLabel Movie::label() const
{
    return m_label;
}

bool Movie::isDuplicate(Movie* movie)
{
    MovieDuplicate md = duplicateProperties(movie);
    return md.imdbId || md.tmdbId || md.title;
}

MovieDuplicate Movie::duplicateProperties(Movie* movie)
{
    MovieDuplicate md;
    md.imdbId = movie->imdbId().isValid() && movie->imdbId() == imdbId();
    md.tmdbId = movie->tmdbId().isValid() && movie->tmdbId() == tmdbId();
    md.title = !movie->name().isEmpty() && movie->name() == name();

    return md;
}

/*** DEBUG ***/

QDebug operator<<(QDebug dbg, const Movie& movie)
{
    QString nl = "\n";
    QString out;
    out.append("Movie").append(nl);
    out.append(QString("  Files:         ").append(nl));
    for (const QString& file : movie.files()) {
        out.append(QString("    %1").arg(file).append(nl));
    }
    out.append(QString("  Name:          ").append(movie.name()).append(nl));
    out.append(QString("  Original-Name: ").append(movie.originalName()).append(nl));
    out.append(QString("  Ratings:").append(nl));
    for (const Rating& rating : movie.ratings()) {
        out.append(
            QString("    %1: %2 (%3 votes)").arg(rating.source).arg(rating.rating).arg(rating.voteCount).append(nl));
    }
    out.append(QString("  Released:      ").append(movie.released().toString("yyyy-MM-dd")).append(nl));
    out.append(QString("  Tagline:       ").append(movie.tagline()).append(nl));
    out.append(QString("  Runtime:       %1").arg(movie.runtime().count()).append(nl));
    out.append(QString("  Certification: ").append(movie.certification().toString()).append(nl));
    out.append(QString("  Playcount:     %1%2").arg(movie.playcount()).arg(nl));
    out.append(QString("  Lastplayed:    ").append(movie.lastPlayed().toString("yyyy-MM-dd HH:mm:ss")).append(nl));
    out.append(QString("  TMDb ID:       ").append(movie.imdbId().toString()).append(nl));
    out.append(QString("  IMDb ID:       ").append(movie.tmdbId().toString()).append(nl));
    out.append(QString("  Set:           ").append(movie.set().name).append(nl));
    out.append(QString("  Overview:      ").append(movie.overview())).append(nl);
    for (const QString& studio : movie.studios()) {
        out.append(QString("  Studio:         ").append(studio)).append(nl);
    }
    for (const QString& genre : movie.genres()) {
        out.append(QString("  Genre:         ").append(genre)).append(nl);
    }
    for (const QString& country : movie.countries()) {
        out.append(QString("  Country:       ").append(country)).append(nl);
    }
    for (const Actor& actor : movie.actors()) {
        out.append(QString("  Actor:         ").append(nl));
        out.append(QString("    Name:  ").append(actor.name)).append(nl);
        out.append(QString("    Role:  ").append(actor.role)).append(nl);
        out.append(QString("    Thumb: ").append(actor.thumb)).append(nl);
    }
    for (const Poster& poster : movie.constImages().posters()) {
        out.append(QString("  Poster:       ")).append(nl);
        out.append(QString("    ID:       ").append(poster.id)).append(nl);
        out.append(QString("    Original: ").append(poster.originalUrl.toString())).append(nl);
        out.append(QString("    Thumb:    ").append(poster.thumbUrl.toString())).append(nl);
    }
    for (const Poster& backdrop : movie.constImages().backdrops()) {
        out.append(QString("  Backdrop:       ")).append(nl);
        out.append(QString("    ID:       ").append(backdrop.id)).append(nl);
        out.append(QString("    Original: ").append(backdrop.originalUrl.toString())).append(nl);
        out.append(QString("    Thumb:    ").append(backdrop.thumbUrl.toString())).append(nl);
    }
    dbg.nospace() << out;
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const Movie* movie)
{
    dbg.nospace() << *movie;
    return dbg.space();
}
