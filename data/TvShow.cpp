#include "TvShow.h"
#include "globals/Globals.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include "globals/Globals.h"

/**
 * @brief TvShow::TvShow
 * @param dir
 * @param parent
 */
TvShow::TvShow(QString dir, QObject *parent) :
    QObject(parent)
{
    m_dir = dir;
    m_infoLoaded = false;
    m_backdropImageChanged = false;
    m_posterImageChanged = false;
    m_bannerImageChanged = false;
    m_logoImageChanged = false;
    m_clearArtImageChanged = false;
    m_characterArtImageChanged = false;
    m_hasChanged = false;
    clear();
    m_downloadsInProgress = false;
    static int m_idCounter = 0;
    m_showId = ++m_idCounter;
}

/**
 * @brief Moves this object and all child items to the main thread
 */
void TvShow::moveToMainThread()
{
    moveToThread(QApplication::instance()->thread());
    for (int i=0, n=m_episodes.count() ; i<n ; ++i)
        m_episodes[i]->moveToMainThread();
}

/**
 * @brief Clears all data
 */
void TvShow::clear()
{
    m_showTitle = "";
    m_rating = 0;
    m_firstAired = QDate(2000, 02, 30); // invalid date
    m_certification = "";
    m_network = "";
    m_overview = "";
    m_genres.clear();
    m_actors.clear();
    m_backdrops.clear();
    m_posters.clear();
    m_banners.clear();
    m_seasonPosters.clear();
    m_backdropImageChanged = false;
    m_posterImageChanged = false;
    m_bannerImageChanged = false;
    m_logoImageChanged = false;
    m_clearArtImageChanged = false;
    m_characterArtImageChanged = false;
    m_seasonPosterImages.clear();
    m_seasonPosterImagesChanged.clear();
    m_hasChanged = false;
}

/**
 * @brief Adds an episode
 * @param episode Episode to add
 */
void TvShow::addEpisode(TvShowEpisode *episode)
{
    m_episodes.append(episode);
}

/**
 * @brief TvShow::episodeCount
 * @return Number of child episodes
 */
int TvShow::episodeCount()
{
    return m_episodes.count();
}

/**
 * @brief Load data using the given MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use
 * @return Loading was successful or not
 */
bool TvShow::loadData(MediaCenterInterface *mediaCenterInterface)
{
    qDebug() << "Entered";
    bool infoLoaded = mediaCenterInterface->loadTvShow(this);
    qDebug() << "Loaded" << infoLoaded;
    if (!infoLoaded) {
        QStringList dirParts = this->dir().split(QDir::separator());
        if (dirParts.count() > 0)
            this->setName(dirParts.last());
    }
    m_infoLoaded = infoLoaded;
    setChanged(false);
    return infoLoaded;
}

/**
 * @brief Loads the shows data using a scraper
 * @param id ID of the show for the given scraper
 * @param tvScraperInterface Scraper to use
 * @param updateAllEpisodes Force update all child episodes (regardless if they already have infos)
 */
void TvShow::loadData(QString id, TvScraperInterface *tvScraperInterface, bool updateAllEpisodes)
{
    qDebug() << "Entered, id=" << id << "scraperInterface" << tvScraperInterface->name() << "updateAllEpisodes" << updateAllEpisodes;
    if (tvScraperInterface->name() == "The TV DB")
        setTvdbId(id);
    tvScraperInterface->loadTvShowData(id, this, updateAllEpisodes);
}

/**
 * @brief Saves the shows data
 * @param mediaCenterInterface MediaCenterInterface to use
 * @return Saving was successful
 */
bool TvShow::saveData(MediaCenterInterface *mediaCenterInterface)
{
    qDebug() << "Entered";
    bool saved = mediaCenterInterface->saveTvShow(this);
    if (!m_infoLoaded)
        m_infoLoaded = saved;

    setChanged(false);
    clearImages();
    return saved;
}

/**
 * @brief Called from the scraper interface
 */
void TvShow::scraperLoadDone()
{
    emit sigLoaded(this);
}

/**
 * @brief Clears the movie images to save memory
 */
void TvShow::clearImages()
{
    m_posterImage = QImage();
    m_backdropImage = QImage();
    m_bannerImage = QImage();
    m_logoImage = QImage();
    m_clearArtImage = QImage();
    m_characterArtImage = QImage();
    foreach (int season, seasons())
        m_seasonPosterImages[season] = QImage();
    foreach (Actor *actor, actorsPointer())
        actor->image = QImage();
}

/**
 * @brief TvShow::hasNewEpisodes
 * @return
 */
bool TvShow::hasNewEpisodes() const
{
    foreach (TvShowEpisode *episode, m_episodes) {
        if (!episode->infoLoaded())
            return true;
    }
    return false;
}

/**
 * @brief TvShow::hasNewEpisodesInSeason
 * @param season Season number
 * @return
 */
bool TvShow::hasNewEpisodesInSeason(QString season) const
{
    foreach (TvShowEpisode *episode, m_episodes) {
        if (episode->season() == season.toInt() && !episode->infoLoaded())
            return true;
    }
    return false;
}

/*** GETTER ***/

/**
 * @brief TvShow::infoLoaded
 * @return
 */
bool TvShow::infoLoaded() const
{
    return m_infoLoaded;
}

/**
 * @brief TvShow::dir
 * @return
 */
QString TvShow::dir() const
{
    return m_dir;
}

/**
 * @property TvShow::name
 * @brief Name of the show
 * @return Name
 * @see TvShow::setName
 */
QString TvShow::name() const
{
    return m_name;
}

/**
 * @property TvShow::showTitle
 * @brief The title of the show
 * @return Title
 * @see TvShow::setShowTitle
 */
QString TvShow::showTitle() const
{
    return m_showTitle;
}

/**
 * @property TvShow::rating
 * @brief Rating of the show
 * @return The rating
 * @see TvShow::setRating
 */
qreal TvShow::rating() const
{
    return m_rating;
}

/**
 * @property TvShow::firstAired
 * @brief First aired date
 * @return Date
 * @see TvShow::setFirstAired
 */
QDate TvShow::firstAired() const
{
    return m_firstAired;
}

/**
 * @property TvShow::genres
 * @brief The genres
 * @return List of genres
 * @see TvShow::addGenre
 * @see TvShow::setGenres
 * @see TvShow::removeGenre
 */
QStringList TvShow::genres() const
{
    return m_genres;
}

/**
 * @brief Constructs a list of pointers to the genres
 * @return List of pointers
 * @see TvShow::genres
 */
QList<QString*> TvShow::genresPointer()
{
    QList<QString*> genres;
    for (int i=0, n=m_genres.size() ; i<n ; ++i)
        genres.append(&m_genres[i]);
    return genres;
}

/**
 * @property TvShow::certification
 * @brief Certification of the show
 * @return Certification
 * @see TvShow::setCertification
 */
QString TvShow::certification() const
{
    return m_certification;
}

/**
 * @property TvShow::network
 * @brief The network
 * @return Network of the show
 * @see TvShow::setNetwork
 */
QString TvShow::network() const
{
    return m_network;
}

/**
 * @property TvShow::overview
 * @brief The plot
 * @return Plot of the show
 * @see TvShow::setOverview
 */
QString TvShow::overview() const
{
    return m_overview;
}

/**
 * @property TvShow::tvdbId
 * @brief TheTvDb Id of the show
 * @return TheTvDb Id
 * @see TvShow::setTvdbId
 */
QString TvShow::tvdbId() const
{
    return m_tvdbId;
}

/**
 * @brief Constructs a list of all certifications used in child episodes
 * @return List of certifications
 */
QStringList TvShow::certifications() const
{
    QStringList certifications;
    foreach (TvShowEpisode *episode, m_episodes) {
        if (!certifications.contains(episode->certification()) && !episode->certification().isEmpty())
            certifications.append(episode->certification());
    }

    return certifications;
}

/**
 * @property TvShow::actors
 * @brief Actors of the show
 * @return List of actors
 * @see TvShow::addActor
 * @see TvShow::setActors
 * @see TvSHow::removeActor
 */
QList<Actor> TvShow::actors() const
{
    return m_actors;
}

/**
 * @brief TvShow::actorsPointer
 * @return List of pointers to actors
 * @see TvShow::addActor
 * @see TvShow::actors
 */
QList<Actor*> TvShow::actorsPointer()
{
    QList<Actor*> actors;
    for (int i=0, n=m_actors.size() ; i<n ; i++)
        actors.append(&(m_actors[i]));
    return actors;
}

/**
 * @property TvShow::posters
 * @brief Tv Show posters
 * @return List of posters
 * @see TvShow::addPoster
 * @see TvShow::setPoster
 * @see TvShow::setPosters
 */
QList<Poster> TvShow::posters() const
{
    return m_posters;
}

/**
 * @property TvShow::banners
 * @brief Banners of the show
 * @return List of all show banners
 * @see TvShow::setBanners
 * @see TvShow::setBanner
 * @see TvShow::addBanner
 */
QList<Poster> TvShow::banners() const
{
    return m_banners;
}

/**
 * @property TvShow::backdrops
 * @brief Backdrops of the tv show
 * @return List of backdrops
 * @see TvShow::setBackdrops
 * @see TvShow::addBackdrop
 * @see TvShow::setBackdrop
 */
QList<Poster> TvShow::backdrops() const
{
    return m_backdrops;
}
/**
 * @brief TvShow::posterImage
 * @return
 */
QImage *TvShow::posterImage()
{
    return &m_posterImage;
}

/**
 * @brief TvShow::backdropImage
 * @return
 */
QImage *TvShow::backdropImage()
{
    return &m_backdropImage;
}

/**
 * @brief TvShow::bannerImage
 * @return
 */
QImage *TvShow::bannerImage()
{
    return &m_bannerImage;
}

/**
 * @brief TvShow::logoImage
 * @return
 */
QImage *TvShow::logoImage()
{
    return &m_logoImage;
}

/**
 * @brief TvShow::clearArtImage
 * @return
 */
QImage *TvShow::clearArtImage()
{
    return &m_clearArtImage;
}

/**
 * @brief TvShow::characterArtImage
 * @return
 */
QImage *TvShow::characterArtImage()
{
    return &m_characterArtImage;
}

/**
 * @brief TvShow::seasonPosterImage
 * @param season
 * @return
 */
QImage *TvShow::seasonPosterImage(int season)
{
    if (!m_seasonPosterImages.contains(season))
        m_seasonPosterImages.insert(season, QImage());

    return &m_seasonPosterImages[season];
}

/**
 * @brief TvShow::seasonPosters
 * @param season
 * @return
 */
QList<Poster> TvShow::seasonPosters(int season) const
{
    if (!m_seasonPosters.contains(season))
        return QList<Poster>();

    return m_seasonPosters[season];
}

/**
 * @brief TvShow::episode
 * @param season
 * @param episode
 * @return Episode object
 */
TvShowEpisode *TvShow::episode(int season, int episode)
{
    for (int i=0, n=m_episodes.count() ; i<n ; ++i) {
        if (m_episodes[i]->season() == season && m_episodes[i]->episode() == episode)
            return m_episodes[i];
    }
    return new TvShowEpisode(QStringList(), this);
}

/**
 * @brief TvShow::posterImageChanged
 * @return
 */
bool TvShow::posterImageChanged() const
{
    return m_posterImageChanged;
}

/**
 * @brief TvShow::backdropImageChanged
 * @return
 */
bool TvShow::backdropImageChanged() const
{
    return m_backdropImageChanged;
}

/**
 * @brief TvShow::bannerImageChanged
 * @return
 */
bool TvShow::bannerImageChanged() const
{
    return m_bannerImageChanged;
}

/**
 * @brief TvShow::logoImageChanged
 * @return
 */
bool TvShow::logoImageChanged() const
{
    return m_logoImageChanged;
}

/**
 * @brief TvShow::clearArtImageChanged
 * @return
 */
bool TvShow::clearArtImageChanged() const
{
    return m_clearArtImageChanged;
}

/**
 * @brief TvShow::characterArtImageChanged
 * @return
 */
bool TvShow::characterArtImageChanged() const
{
    return m_characterArtImageChanged;
}

/**
 * @brief TvShow::seasonPosterImageChanged
 * @param season
 * @return
 */
bool TvShow::seasonPosterImageChanged(int season) const
{
    return m_seasonPosterImagesChanged.contains(season);
}

/**
 * @brief TvShow::seasons
 * @return
 */
QList<int> TvShow::seasons()
{
    QList<int> seasons;
    foreach (TvShowEpisode *episode, m_episodes) {
        if (!seasons.contains(episode->season()) && episode->season() != -2)
            seasons.append(episode->season());
    }
    return seasons;
}

/**
 * @brief TvShow::episodes
 * @return
 */
QList<TvShowEpisode*> TvShow::episodes()
{
    return m_episodes;
}

/**
 * @brief TvShow::modelItem
 * @return
 */
TvShowModelItem *TvShow::modelItem()
{
    return m_modelItem;
}

/**
 * @brief Returns true if something has changed since the last load
 * @return
 */
bool TvShow::hasChanged() const
{
    return m_hasChanged;
}

/**
 * @brief TvShow::mediaCenterPath
 * @return
 */
QString TvShow::mediaCenterPath() const
{
    return m_mediaCenterPath;
}

/**
 * @brief TvShow::showId
 * @return
 */
int TvShow::showId() const
{
    return m_showId;
}

/**
 * @brief TvShow::downloadsInProgress
 * @return
 */
bool TvShow::downloadsInProgress() const
{
    return m_downloadsInProgress;
}

/*** SETTER ***/

/**
 * @brief Sets the name of the episode
 * @param name
 * @see TvShow::name
 */
void TvShow::setName(QString name)
{
    m_name = name;
    setChanged(true);
}

/**
 * @brief Sets the show title
 * @param title
 * @see TvShow::showTitle
 */
void TvShow::setShowTitle(QString title)
{
    m_showTitle = title;
    setChanged(true);
}

/**
 * @brief Sets the rating
 * @param rating
 * @see TvShow::rating
 */
void TvShow::setRating(qreal rating)
{
    m_rating = rating;
    setChanged(true);
}

/**
 * @brief Sets the first aired date
 * @param aired
 * @see TvShow::firstAired
 */
void TvShow::setFirstAired(QDate aired)
{
    m_firstAired = aired;
    setChanged(true);
}

/**
 * @brief Sets all genres
 * @param genres
 * @see TvShow::genres
 */
void TvShow::setGenres(QStringList genres)
{
    m_genres = genres;
    setChanged(true);
}

/**
 * @brief Adds a genre
 * @param genre
 * @see TvShow::genres
 */
void TvShow::addGenre(QString genre)
{
    m_genres.append(genre);
    setChanged(true);
}

/**
 * @brief Sets the certification
 * @param certification
 * @see TvShow::certification
 */
void TvShow::setCertification(QString certification)
{
    m_certification = certification;
    setChanged(true);
}

/**
 * @brief Sets the network
 * @param network
 * @see TvShow::network
 */
void TvShow::setNetwork(QString network)
{
    m_network = network;
    setChanged(true);
}

/**
 * @brief Sets the plot
 * @param overview
 * @see TvShow::overview
 */
void TvShow::setOverview(QString overview)
{
    m_overview = overview;
    setChanged(true);
}

/**
 * @brief Sets the TheTvdbId
 * @param id
 * @see TvShow::tvdbId
 */
void TvShow::setTvdbId(QString id)
{
    m_tvdbId = id;
    setChanged(true);
}

/**
 * @brief Sets all actors
 * @param actors
 * @see TvShow::actors
 */
void TvShow::setActors(QList<Actor> actors)
{
    m_actors = actors;
    setChanged(true);
}

/**
 * @brief Adds an actor
 * @param actor
 * @see TvShow::actors
 */
void TvShow::addActor(Actor actor)
{
    m_actors.append(actor);
    setChanged(true);
}

/**
 * @brief Sets all posters
 * @param posters
 * @see TvShow::posters
 */
void TvShow::setPosters(QList<Poster> posters)
{
    m_posters = posters;
    setChanged(true);
}

/**
 * @brief Sets all banners
 * @param banners
 * @see TvShow::banners
 */
void TvShow::setBanners(QList<Poster> banners)
{
    m_banners = banners;
    setChanged(true);
}

/**
 * @brief Sets the poster for a specific index
 * @param index
 * @param poster
 * @see TvShow::posters
 */
void TvShow::setPoster(int index, Poster poster)
{
    if (m_posters.size() < index)
        return;
    m_posters[index] = poster;
    setChanged(true);
}

/**
 * @brief Appends a list of backdrops
 * @param backdrops
 * @see TvShow::backdrops
 */
void TvShow::setBackdrops(QList<Poster> backdrops)
{
    m_backdrops.append(backdrops);
    setChanged(true);
}

/**
 * @brief Sets the backdrop for a specific index
 * @param index
 * @param backdrop
 * @see TvShow::backdrops
 */
void TvShow::setBackdrop(int index, Poster backdrop)
{
    if (m_backdrops.size() < index)
        return;
    m_backdrops[index] = backdrop;
    setChanged(true);
}

/**
 * @brief Adds a poster
 * @param poster
 * @see TvShow::posters
 */
void TvShow::addPoster(Poster poster)
{
    m_posters.append(poster);
    setChanged(true);
}

/**
 * @brief Adds a banner
 * @param banner
 * @see TvShow::banners
 */
void TvShow::addBanner(Poster banner)
{
    m_banners.append(banner);
    setChanged(true);
}

/**
 * @brief Adds a backdrop
 * @param backdrop
 * @see TvShow::backdrops
 */
void TvShow::addBackdrop(Poster backdrop)
{
    m_backdrops.append(backdrop);
    setChanged(true);
}

/**
 * @brief Sets the poster image
 * @param poster
 */
void TvShow::setPosterImage(QImage poster)
{
    m_posterImage = QImage(poster);
    m_posterImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the banner image
 * @param banner
 */
void TvShow::setBannerImage(QImage banner)
{
    m_bannerImage = QImage(banner);
    m_bannerImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the logo image
 * @param img
 */
void TvShow::setLogoImage(QImage img)
{
    m_logoImage = QImage(img);
    m_logoImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the clear art image
 * @param img
 */
void TvShow::setClearArtImage(QImage img)
{
    m_clearArtImage = QImage(img);
    m_clearArtImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the character art image
 * @param img
 */
void TvShow::setCharacterArtImage(QImage img)
{
    m_characterArtImage = QImage(img);
    m_characterArtImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the backdrop image
 * @param backdrop
 */
void TvShow::setBackdropImage(QImage backdrop)
{
    m_backdropImage = QImage(backdrop);
    m_backdropImageChanged = true;
    setChanged(true);
}

/**
 * @brief Adds a new season poster
 * @param season
 * @param poster
 */
void TvShow::addSeasonPoster(int season, Poster poster)
{
    if (!m_seasonPosters.contains(season)) {
        QList<Poster> posters;
        m_seasonPosters.insert(season, posters);
    }

    m_seasonPosters[season].append(poster);
    setChanged(true);
}

/**
 * @brief Sets a poster for a season
 * @param season Number of the season
 * @param poster Season poster image
 */
void TvShow::setSeasonPosterImage(int season, QImage poster)
{
    if (m_seasonPosterImages.contains(season))
        m_seasonPosterImages[season] = poster;
    else
        m_seasonPosterImages.insert(season, poster);

    if (!m_seasonPosterImagesChanged.contains(season))
        m_seasonPosterImagesChanged.append(season);
    setChanged(true);
}

/**
 * @brief TvShow::setChanged
 * @param changed
 */
void TvShow::setChanged(bool changed)
{
    m_hasChanged = changed;
    emit sigChanged(this);
}

/**
 * @brief TvShow::setModelItem
 * @param item
 */
void TvShow::setModelItem(TvShowModelItem *item)
{
    m_modelItem = item;
}

/**
 * @brief TvShow::setMediaCenterPath
 * @param path
 */
void TvShow::setMediaCenterPath(QString path)
{
    m_mediaCenterPath = path;
}

/**
 * @brief TvShow::setDownloadsInProgress
 * @param inProgress
 */
void TvShow::setDownloadsInProgress(bool inProgress)
{
    m_downloadsInProgress = inProgress;
}

/*** REMOVER ***/

/**
 * @brief Removes an actor
 * @param actor Pointer to the actor to remove
 * @see TvShow::actors
 */
void TvShow::removeActor(Actor *actor)
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
 * @brief Removes a genre
 * @param genre Pointer to the genre to remove
 * @see TvShow::genres
 */
void TvShow::removeGenre(QString *genre)
{
    for (int i=0, n=m_genres.size() ; i<n ; ++i) {
        if (&m_genres[i] == genre) {
            m_genres.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

/*** DEBUG ***/

QDebug operator<<(QDebug dbg, const TvShow &show)
{
    QString nl = "\n";
    QString out;
    out.append("TvShow").append(nl);
    out.append(QString("  Dir:           ").append(show.dir()).append(nl));
    out.append(QString("  Name:          ").append(show.name()).append(nl));
    out.append(QString("  ShowTitle:     ").append(show.showTitle()).append(nl));
    out.append(QString("  Rating:        %1").arg(show.rating()).append(nl));
    out.append(QString("  FirstAired:    ").append(show.firstAired().toString("yyyy-MM-dd")).append(nl));
    out.append(QString("  Certification: ").append(show.certification()).append(nl));
    out.append(QString("  Network:       ").append(show.network()).append(nl));
    out.append(QString("  Overview:      ").append(show.overview())).append(nl);
    foreach (const QString &genre, show.genres())
        out.append(QString("  Genre:         ").append(genre)).append(nl);
    foreach (const Actor &actor, show.actors()) {
        out.append(QString("  Actor:         ").append(nl));
        out.append(QString("    Name:  ").append(actor.name)).append(nl);
        out.append(QString("    Role:  ").append(actor.role)).append(nl);
        out.append(QString("    Thumb: ").append(actor.thumb)).append(nl);
    }
    /*
    foreach (const QString &studio, movie.studios())
        out.append(QString("  Studio:         ").append(studio)).append(nl);
    foreach (const QString &country, movie.countries())
        out.append(QString("  Country:       ").append(country)).append(nl);
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
    */
    dbg.nospace() << out;
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const TvShow *show)
{
    dbg.nospace() << *show;
    return dbg.space();
}
