#include "Movie.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

Movie::Movie(QStringList files, QObject *parent) :
    QObject(parent)
{
    moveToThread(QApplication::instance()->thread());
    m_files = files;
    m_rating = 0;
    m_runtime = 0;
    m_playcount = 0;
    m_backdropImageChanged = false;
    m_posterImageChanged = false;
    if (files.size() > 0) {
        QFileInfo fi(files.at(0));
        QStringList path = fi.path().split("/", QString::SkipEmptyParts);
        if (!path.isEmpty())
            m_folderName = path.last();
    }
    m_infoLoaded = false;
    m_imagesLoaded = false;
    m_watched = false;
    m_hasChanged = false;
    m_downloadsInProgress = false;
    m_inSeparateFolder = false;
    static int m_idCounter = 0;
    m_movieId = ++m_idCounter;
}

Movie::~Movie()
{
}

void Movie::clear()
{
    m_hasChanged = false;
    m_actors.clear();
    m_backdrops.clear();
    m_countries.clear();
    m_genres.clear();
    m_posters.clear();
    m_studios.clear();
    m_originalName = "";
    m_overview = "";
    m_rating = 0;
    m_released = QDate(2000, 02, 30); // invalid date
    m_tagline = "";
    m_runtime = 0;
    m_trailer = "";
    m_certification = "";
    m_sortTitle = "";
}

bool Movie::saveData(MediaCenterInterface *mediaCenterInterface)
{
    bool saved = mediaCenterInterface->saveMovie(this);
    if (!m_infoLoaded)
        m_infoLoaded = saved;
    setChanged(false);
    return saved;
}

bool Movie::loadData(MediaCenterInterface *mediaCenterInterface)
{
    if (m_infoLoaded || hasChanged())
        return m_infoLoaded;

    bool infoLoaded = mediaCenterInterface->loadMovie(this);
    if (!infoLoaded) {
        if (this->files().size() > 0) {
            QFileInfo fi(this->files().at(0));
            if (QString::compare(fi.fileName(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = fi.canonicalPath().split(QDir::separator());
                if (pathElements.size() > 0 && QString::compare(pathElements.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0)
                    pathElements.removeLast();
                if (pathElements.size() > 0)
                    this->setName(pathElements.last());
            } else if (QString::compare(fi.fileName(), "index.bdmv", Qt::CaseInsensitive) == 0) {
                    QStringList pathElements = fi.canonicalPath().split(QDir::separator());
                    if (pathElements.size() > 0 && QString::compare(pathElements.last(), "BDMV", Qt::CaseInsensitive) == 0)
                        pathElements.removeLast();
                    if (pathElements.size() > 0)
                        this->setName(pathElements.last());
            } else if (inSeparateFolder()) {
                QStringList splitted = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!splitted.isEmpty())
                    setName(splitted.last());
                else
                    setName(fi.completeBaseName().replace(".", " ").replace("_", " "));
            } else {
                setName(fi.completeBaseName().replace(".", " ").replace("_", " "));
            }
        }
    }
    m_infoLoaded = infoLoaded;
    setChanged(false);
    return infoLoaded;
}

void Movie::loadData(QString id, ScraperInterface *scraperInterface)
{
    scraperInterface->loadData(id, this);
}

void Movie::scraperLoadDone()
{
    emit loaded(this);
}

void Movie::loadImages(MediaCenterInterface *mediaCenterInterface)
{
    if (!m_imagesLoaded)
        mediaCenterInterface->loadMovieImages(this);
    m_imagesLoaded = true;
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

QString Movie::overview() const
{
    return m_overview;
}

qreal Movie::rating() const
{
    return m_rating;
}

QDate Movie::released() const
{
    return m_released;
}

QString Movie::tagline() const
{
    return m_tagline;
}

int Movie::runtime() const
{
    return m_runtime;
}

QString Movie::certification() const
{
    return m_certification;
}

QStringList Movie::genres() const
{
    return m_genres;
}

QList<QString*> Movie::genresPointer()
{
    QList<QString*> genres;
    for (int i=0, n=m_genres.size() ; i<n ; ++i)
        genres.append(&m_genres[i]);
    return genres;
}

QStringList Movie::countries() const
{
    return m_countries;
}

QList<QString*> Movie::countriesPointer()
{
    QList<QString*> countries;
    for (int i=0, n=m_countries.size() ; i<n ; ++i)
        countries.append(&m_countries[i]);
    return countries;
}

QStringList Movie::studios() const
{
    return m_studios;
}

QList<QString*> Movie::studiosPointer()
{
    QList<QString*> studios;
    for (int i=0, n=m_studios.size() ; i<n ; ++i)
        studios.append(&m_studios[i]);
    return studios;
}

QUrl Movie::trailer() const
{
    return m_trailer;
}

QList<Actor> Movie::actors() const
{
    return m_actors;
}

QList<Actor*> Movie::actorsPointer()
{
    QList<Actor*> actors;
    for (int i=0, n=m_actors.size() ; i<n ; i++)
        actors.append(&(m_actors[i]));
    return actors;
}

QStringList Movie::files() const
{
    return m_files;
}

int Movie::playcount() const
{
    return m_playcount;
}

QDateTime Movie::lastPlayed() const
{
    return m_lastPlayed;
}

QString Movie::id() const
{
    return m_id;
}

QString Movie::set() const
{
    return m_set;
}

QList<Poster> Movie::posters() const
{
    return m_posters;
}

QList<Poster> Movie::backdrops() const
{
    return m_backdrops;
}

QImage *Movie::posterImage()
{
    return &m_posterImage;
}

QImage *Movie::backdropImage()
{
    return &m_backdropImage;
}

QString Movie::folderName() const
{
    return m_folderName;
}

bool Movie::infoLoaded() const
{
    return m_infoLoaded;
}

bool Movie::posterImageChanged() const
{
    return m_posterImageChanged;
}

bool Movie::backdropImageChanged() const
{
    return m_backdropImageChanged;
}

bool Movie::watched() const
{
    return m_watched;
}

bool Movie::hasChanged() const
{
    return m_hasChanged;
}

int Movie::movieId() const
{
    return m_movieId;
}

bool Movie::downloadsInProgress() const
{
    return m_downloadsInProgress;
}

bool Movie::inSeparateFolder() const
{
    return m_inSeparateFolder;
}

/*** SETTER ***/

void Movie::setName(QString name)
{
    m_name = name;
    setChanged(true);
}

void Movie::setSortTitle(QString sortTitle)
{
    m_sortTitle = sortTitle;
    setChanged(true);
}

void Movie::setOriginalName(QString originalName)
{
    m_originalName = originalName;
    setChanged(true);
}

void Movie::setOverview(QString overview)
{
    m_overview = overview;
    setChanged(true);
}

void Movie::setRating(qreal rating)
{
    m_rating = rating;
    setChanged(true);
}

void Movie::setReleased(QDate released)
{
    m_released = released;
    setChanged(true);
}

void Movie::setTagline(QString tagline)
{
    m_tagline = tagline;
    setChanged(true);
}

void Movie::setRuntime(int runtime)
{
    m_runtime = runtime;
    setChanged(true);
}

void Movie::setCertification(QString certification)
{
    m_certification = certification;
    setChanged(true);
}

void Movie::setGenres(QStringList genres)
{
    m_genres = genres;
    setChanged(true);
}

void Movie::setCountries(QStringList countries)
{
    m_countries = countries;
    setChanged(true);
}

void Movie::setStudios(QStringList studios)
{
    m_studios = studios;
    setChanged(true);
}

void Movie::setTrailer(QUrl trailer)
{
    m_trailer = trailer;
    setChanged(true);
}

void Movie::setActors(QList<Actor> actors)
{
    m_actors = actors;
    setChanged(true);
}

void Movie::setPlayCount(int playcount)
{
    m_playcount = playcount;
    setChanged(true);
}

void Movie::setLastPlayed(QDateTime lastPlayed)
{
    m_lastPlayed = lastPlayed;
    setChanged(true);
}

void Movie::setId(QString id)
{
    m_id = id;
    setChanged(true);
}

void Movie::setSet(QString set)
{
    m_set = set;
    setChanged(true);
}

void Movie::setPosters(QList<Poster> posters)
{
    m_posters = posters;
    setChanged(true);
}

void Movie::setPoster(int index, Poster poster)
{
    if (m_posters.size() < index)
        return;
    m_posters[index] = poster;
    setChanged(true);
}

void Movie::setBackdrops(QList<Poster> backdrops)
{
    m_backdrops.append(backdrops);
    setChanged(true);
}

void Movie::setBackdrop(int index, Poster backdrop)
{
    if (m_backdrops.size() < index)
        return;
    m_backdrops[index] = backdrop;
    setChanged(true);
}

void Movie::setWatched(bool watched)
{
    m_watched = watched;
    setChanged(true);
}

void Movie::setChanged(bool changed)
{
    m_hasChanged = changed;
    emit sigChanged(this);
}

void Movie::setDownloadsInProgress(bool inProgress)
{
    m_downloadsInProgress = inProgress;
}

void Movie::setInSeparateFolder(bool inSepFolder)
{
    m_inSeparateFolder = inSepFolder;
}

/*** ADDER ***/

void Movie::addActor(Actor actor)
{
    m_actors.append(actor);
    setChanged(true);
}

void Movie::addCountry(QString country)
{
    m_countries.append(country);
    setChanged(true);
}

void Movie::addGenre(QString genre)
{
    m_genres.append(genre);
    setChanged(true);
}

void Movie::addStudio(QString studio)
{
    m_studios.append(studio);
    setChanged(true);
}

void Movie::addPoster(Poster poster)
{
    m_posters.append(poster);
    setChanged(true);
}

void Movie::addBackdrop(Poster backdrop)
{
    m_backdrops.append(backdrop);
    setChanged(true);
}

void Movie::setPosterImage(QImage poster)
{
    m_posterImage = QImage(poster);
    m_posterImageChanged = true;
    setChanged(true);
}

void Movie::setBackdropImage(QImage backdrop)
{
    m_backdropImage = QImage(backdrop);
    m_backdropImageChanged = true;
    setChanged(true);
}

/*** REMOVER ***/

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
