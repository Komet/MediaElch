#include "TvShow.h"
#include "globals/Globals.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/NameFormatter.h"

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
    m_infoFromNfoLoaded = false;
    m_hasChanged = false;
    clear();
    m_downloadsInProgress = false;
    static int m_idCounter = 0;
    m_showId = ++m_idCounter;
    m_databaseId = -1;
    m_syncNeeded = false;
    m_hasTune = false;
    m_runtime = 0;
    m_showMissingEpisodes = false;
    m_hideSpecialsInMissingEpisodes = false;
    m_votes = 0;
    m_top250 = 0;
}

/**
 * @brief Clears all data
 */
void TvShow::clear()
{
    QList<int> infos;
    infos << TvShowScraperInfos::Actors
          << TvShowScraperInfos::Banner
          << TvShowScraperInfos::Certification
          << TvShowScraperInfos::Fanart
          << TvShowScraperInfos::FirstAired
          << TvShowScraperInfos::Genres
          << TvShowScraperInfos::Network
          << TvShowScraperInfos::Overview
          << TvShowScraperInfos::Poster
          << TvShowScraperInfos::Rating
          << TvShowScraperInfos::SeasonPoster
          << TvShowScraperInfos::Title
          << TvShowScraperInfos::Tags
          << TvShowScraperInfos::ExtraArts
          << TvShowScraperInfos::ExtraFanarts
          << TvShowScraperInfos::Thumb
          << TvShowScraperInfos::SeasonThumb
          << TvShowScraperInfos::Runtime;
    clear(infos);
    m_nfoContent.clear();
}

void TvShow::clear(QList<int> infos)
{
    if (infos.contains(TvShowScraperInfos::Actors))
        m_actors.clear();
    if (infos.contains(TvShowScraperInfos::Banner)) {
        m_banners.clear();
        m_imagesToRemove.remove(ImageType::TvShowBanner);
        m_images.insert(ImageType::TvShowBanner, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowBanner, false);
    }
    if (infos.contains(TvShowScraperInfos::Certification))
        m_certification.clear();
    if (infos.contains(TvShowScraperInfos::FirstAired))
        m_firstAired = QDate(2000, 02, 30); // invalid date
    if (infos.contains(TvShowScraperInfos::Genres))
        m_genres.clear();
    if (infos.contains(TvShowScraperInfos::Network))
        m_network.clear();
    if (infos.contains(TvShowScraperInfos::Overview))
        m_overview.clear();
    if (infos.contains(TvShowScraperInfos::Poster)) {
        m_posters.clear();
        m_imagesToRemove.remove(ImageType::TvShowPoster);
        m_images.insert(ImageType::TvShowPoster, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowPoster, false);
    }
    if (infos.contains(TvShowScraperInfos::Rating)) {
        m_rating = 0;
        m_votes = 0;
        m_top250 = 0;
    }
    if (infos.contains(TvShowScraperInfos::SeasonPoster)) {
        clearSeasonImageType(ImageType::TvShowSeasonPoster);
        m_seasonPosters.clear();
        m_imagesToRemove.remove(ImageType::TvShowSeasonPoster);
    }
    if (infos.contains(TvShowScraperInfos::SeasonBackdrop)) {
        clearSeasonImageType(ImageType::TvShowSeasonBackdrop);
        m_seasonBackdrops.clear();
        m_imagesToRemove.remove(ImageType::TvShowSeasonBackdrop);
    }
    if (infos.contains(TvShowScraperInfos::SeasonBanner)) {
        clearSeasonImageType(ImageType::TvShowSeasonBanner);
        m_seasonBanners.clear();
        m_imagesToRemove.remove(ImageType::TvShowSeasonBanner);
    }
    if (infos.contains(TvShowScraperInfos::SeasonThumb)) {
        clearSeasonImageType(ImageType::TvShowSeasonThumb);
        m_seasonThumbs.clear();
        m_imagesToRemove.remove(ImageType::TvShowSeasonThumb);
    }
    if (infos.contains(TvShowScraperInfos::Title))
        m_showTitle.clear();
    if (infos.contains(TvShowScraperInfos::Tags))
        m_tags.clear();
    if (infos.contains(TvShowScraperInfos::Fanart)) {
        m_backdrops.clear();
        m_imagesToRemove.remove(ImageType::TvShowBackdrop);
        m_images.insert(ImageType::TvShowBackdrop, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowBackdrop, false);
    }
    if (infos.contains(TvShowScraperInfos::ExtraArts)) {
        m_images.insert(ImageType::TvShowLogos, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowLogos, false);
        m_images.insert(ImageType::TvShowThumb, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowThumb, false);
        m_images.insert(ImageType::TvShowClearArt, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowClearArt, false);
        m_images.insert(ImageType::TvShowCharacterArt, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowCharacterArt, false);
        m_imagesToRemove.remove(ImageType::TvShowLogos);
        m_imagesToRemove.remove(ImageType::TvShowClearArt);
        m_imagesToRemove.remove(ImageType::TvShowCharacterArt);
        m_imagesToRemove.remove(ImageType::TvShowThumb);
    }
    if (infos.contains(TvShowScraperInfos::ExtraFanarts)) {
        m_extraFanartsToRemove.clear();
        m_extraFanartImagesToAdd.clear();
        m_extraFanarts.clear();
    }
    if (infos.contains(TvShowScraperInfos::Runtime))
        m_runtime = 0;
    m_hasChanged = false;
}

void TvShow::clearSeasonImageType(int imageType)
{
    QMapIterator<int, QMap<int, QByteArray> > it(m_seasonImages);
    while (it.hasNext()) {
        it.next();
        m_seasonImages[it.key()].insert(imageType, QByteArray());
    }
    QMapIterator<int, QMap<int, bool> > itC(m_hasSeasonImageChanged);
    while (itC.hasNext()) {
        itC.next();
        m_hasSeasonImageChanged[itC.key()].insert(imageType, false);
    }

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
bool TvShow::loadData(MediaCenterInterface *mediaCenterInterface, bool reloadFromNfo)
{
    if (hasChanged() || (m_infoLoaded && m_infoFromNfoLoaded && !reloadFromNfo))
        return m_infoLoaded;

    bool infoLoaded;
    if (reloadFromNfo)
        infoLoaded = mediaCenterInterface->loadTvShow(this);
    else
        infoLoaded = mediaCenterInterface->loadTvShow(this, nfoContent());

    if (!infoLoaded) {
        QStringList dirParts = this->dir().split(QDir::separator());
        if (dirParts.count() > 0)
            setName(NameFormatter::instance()->formatName(dirParts.last()));
    }
    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    setChanged(false);

    m_hasImage.clear();
    foreach (const int &imageType, TvShow::imageTypes())
        m_hasImage.insert(imageType, !mediaCenterInterface->imageFileName(this, imageType).isEmpty());
    m_hasImage.insert(ImageType::TvShowExtraFanart, !mediaCenterInterface->extraFanartNames(this).isEmpty());

    return infoLoaded;
}

/**
 * @brief Loads the shows data using a scraper
 * @param id ID of the show for the given scraper
 * @param tvScraperInterface Scraper to use
 */
void TvShow::loadData(QString id, TvScraperInterface *tvScraperInterface, TvShowUpdateType type, QList<int> infosToLoad)
{
    if (tvScraperInterface->identifier() == "tvdb")
        setTvdbId(id);
    m_infosToLoad = infosToLoad;
    tvScraperInterface->loadTvShowData(id, this, type, infosToLoad);
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
    setSyncNeeded(true);
    clearImages();
    clearExtraFanartData();
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
    m_images.clear();
    m_seasonImages.clear();
    m_hasImageChanged.clear();
    m_hasSeasonImageChanged.clear();
    foreach (Actor *actor, actorsPointer())
        actor->image = QByteArray();
    m_extraFanartImagesToAdd.clear();
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

QList<int> TvShow::infosToLoad() const
{
    return m_infosToLoad;
}

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

QString TvShow::id() const
{
    return m_id;
}

QString TvShow::imdbId() const
{
    return m_imdbId;
}

/**
 * @property TvShow::episodeGuideUrl
 * @brief The Episode Guide url of the show
 * @return Episode guide url
 * @see TvShow::setEpisodeGuideUrl
 */
QString TvShow::episodeGuideUrl() const
{
    return m_episodeGuideUrl;
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

QList<Poster> TvShow::seasonBackdrops(int season) const
{
    if (!m_seasonBackdrops.contains(season))
        return QList<Poster>();

    return m_seasonBackdrops[season];
}

QList<Poster> TvShow::seasonBanners(int season, bool returnAll) const
{
    if (!m_seasonBanners.contains(season) && !returnAll)
        return QList<Poster>();

    if (!returnAll)
        return m_seasonBanners[season];

    QList<Poster> banners;
    if (m_seasonBanners.contains(season))
        banners = m_seasonBanners[season];

    QMapIterator<int, QList<Poster> > it(m_seasonBanners);
    while (it.hasNext()) {
        it.next();
        if (it.key() == season)
            continue;
        banners << it.value();
    }
    return banners;
}

QList<Poster> TvShow::seasonThumbs(int season, bool returnAll) const
{
    if (!m_seasonThumbs.contains(season) && !returnAll)
        return QList<Poster>();

    if (!returnAll)
        return m_seasonThumbs[season];

    QList<Poster> thumbs;
    if (m_seasonThumbs.contains(season))
        thumbs = m_seasonThumbs[season];

    QMapIterator<int, QList<Poster> > it(m_seasonThumbs);
    while (it.hasNext()) {
        it.next();
        if (it.key() == season)
            continue;
        thumbs << it.value();
    }
    return thumbs;
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
 * @brief TvShow::seasons
 * @return
 */
QList<int> TvShow::seasons(bool includeDummies)
{
    QList<int> seasons;
    foreach (TvShowEpisode *episode, m_episodes) {
        if (episode->isDummy() && !includeDummies)
            continue;
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

QList<TvShowEpisode*> TvShow::episodes(int season)
{
    QList<TvShowEpisode*> episodes;
    foreach (TvShowEpisode *episode, m_episodes) {
        if (episode->season() == season)
            episodes << episode;
    }

    return episodes;
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

/**
 * @brief TvShow::nfoContent
 * @return
 */
QString TvShow::nfoContent() const
{
    return m_nfoContent;
}

/**
 * @brief TvShow::databaseId
 * @return
 */
int TvShow::databaseId() const
{
    return m_databaseId;
}

bool TvShow::syncNeeded() const
{
    return m_syncNeeded;
}

QStringList TvShow::tags() const
{
    return m_tags;
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
    m_genres.clear();
    foreach (const QString &genre, genres) {
        if (!genre.isEmpty())
            m_genres.append(genre);
    }

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
    if (genre.isEmpty())
        return;
    m_genres.append(genre);
    setChanged(true);
}

void TvShow::addTag(QString tag)
{
    m_tags.append(tag);
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
    if (m_id.isEmpty())
        m_id = id;
    m_tvdbId = id;
    setChanged(true);
}

void TvShow::setId(QString id)
{
    m_id = id;
    setChanged(true);
}

void TvShow::setImdbId(QString id)
{
    m_imdbId = id;
    setChanged(true);
}

/**
 * @brief Sets the Episode guide url
 * @param url
 * @see TvShow::episodeGuideUrl
 */
void TvShow::setEpisodeGuideUrl(QString url)
{
    m_episodeGuideUrl = url;
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

void TvShow::addSeasonBackdrop(int season, Poster poster)
{
    if (!m_seasonBackdrops.contains(season)) {
        QList<Poster> posters;
        m_seasonBackdrops.insert(season, posters);
    }

    m_seasonBackdrops[season].append(poster);
    setChanged(true);
}

void TvShow::addSeasonBanner(int season, Poster poster)
{
    if (!m_seasonBanners.contains(season)) {
        QList<Poster> posters;
        m_seasonBanners.insert(season, posters);
    }

    m_seasonBanners[season].append(poster);
    setChanged(true);
}

void TvShow::addSeasonThumb(int season, Poster poster)
{
    if (!m_seasonThumbs.contains(season)) {
        QList<Poster> posters;
        m_seasonThumbs.insert(season, posters);
    }

    m_seasonThumbs[season].append(poster);
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
 * @param genre Genre to remove
 * @see TvShow::genres
 */
void TvShow::removeGenre(QString genre)
{
    m_genres.removeAll(genre);
    setChanged(true);
}

void TvShow::removeTag(QString tag)
{
    m_tags.removeAll(tag);
    setChanged(true);
}

/**
 * @brief TvShow::setNfoContent
 * @param content
 */
void TvShow::setNfoContent(QString content)
{
    m_nfoContent = content;
}

/**
 * @brief TvShow::setDatabaseId
 * @param id
 */
void TvShow::setDatabaseId(int id)
{
    m_databaseId = id;
}

void TvShow::setSyncNeeded(bool syncNeeded)
{
    m_syncNeeded = syncNeeded;
}

void TvShow::addExtraFanart(QByteArray fanart)
{
    m_extraFanartImagesToAdd.append(fanart);
    setChanged(true);
}

void TvShow::removeExtraFanart(QByteArray fanart)
{
    m_extraFanartImagesToAdd.removeOne(fanart);
    setChanged(true);
}

void TvShow::removeExtraFanart(QString file)
{
    m_extraFanarts.removeOne(file);
    m_extraFanartsToRemove.append(file);
    setChanged(true);
}

QList<ExtraFanart> TvShow::extraFanarts(MediaCenterInterface *mediaCenterInterface)
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

QStringList TvShow::extraFanartsToRemove()
{
    return m_extraFanartsToRemove;
}

QList<QByteArray> TvShow::extraFanartImagesToAdd()
{
    return m_extraFanartImagesToAdd;
}

void TvShow::clearExtraFanartData()
{
    m_extraFanartImagesToAdd.clear();
    m_extraFanartsToRemove.clear();
    m_extraFanarts.clear();
}

QMap<int, QList<int> > TvShow::imagesToRemove() const
{
    return m_imagesToRemove;
}

void TvShow::removeImage(int type, int season)
{
    if (TvShow::seasonImageTypes().contains(type)) {
        if (m_seasonImages.contains(season) && !m_seasonImages.value(season).value(type, QByteArray()).isNull()) {
            m_seasonImages[season].insert(type, QByteArray());
            if (!m_hasSeasonImageChanged.contains(season))
                m_hasSeasonImageChanged.insert(season, QMap<int, bool>());
            m_hasSeasonImageChanged[season].insert(type, false);
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << season);
        } else if (m_imagesToRemove.contains(type) && !m_imagesToRemove.value(type).contains(season)) {
            m_imagesToRemove[type].append(season);
        }
    } else {
        if (!m_images.value(type, QByteArray()).isNull()) {
            m_images.insert(type, QByteArray());
            m_hasImageChanged.insert(type, false);
        } else {
            m_imagesToRemove.insert(type, QList<int>() << -2);
        }
    }

    setChanged(true);
}

void TvShow::setHasTune(bool hasTune)
{
    m_hasTune = hasTune;
}

bool TvShow::hasTune() const
{
    return m_hasTune;
}

bool TvShow::lessThan(TvShow *a, TvShow *b)
{
    return (QString::localeAwareCompare(Helper::instance()->appendArticle(a->name()), Helper::instance()->appendArticle(b->name())) < 0);
}

QByteArray TvShow::image(int imageType)
{
    return m_images.value(imageType);
}

QByteArray TvShow::seasonImage(int season, int imageType)
{
    if (m_seasonImages.contains(season))
        return m_seasonImages.value(season).value(imageType, QByteArray());
    return QByteArray();
}

void TvShow::setImage(int imageType, QByteArray image)
{
    m_images.insert(imageType, image);
    m_hasImageChanged.insert(imageType, true);
    setChanged(true);
}

void TvShow::setSeasonImage(int season, int imageType, QByteArray image)
{
    if (!m_seasonImages.contains(season))
        m_seasonImages.insert(season, QMap<int, QByteArray>());
    m_seasonImages[season].insert(imageType, image);

    if (!m_hasSeasonImageChanged.contains(season))
        m_hasSeasonImageChanged.insert(season, QMap<int, bool>());
    m_hasSeasonImageChanged[season].insert(imageType, true);
    setChanged(true);
}

bool TvShow::imageHasChanged(int imageType) const
{
    return m_hasImageChanged.value(imageType, false);
}

bool TvShow::seasonImageHasChanged(int season, int imageType) const
{
    if (m_hasSeasonImageChanged.contains(season))
        return m_hasSeasonImageChanged.value(season).value(imageType, false);
    return false;
}

QList<int> TvShow::imageTypes()
{
    return QList<int>() << ImageType::TvShowPoster << ImageType::TvShowCharacterArt
                        << ImageType::TvShowClearArt << ImageType::TvShowLogos
                        << ImageType::TvShowBackdrop << ImageType::TvShowBanner
                        << ImageType::TvShowThumb;
}

QList<int> TvShow::seasonImageTypes()
{
    return QList<int>() << ImageType::TvShowSeasonBackdrop << ImageType::TvShowSeasonBanner
                        << ImageType::TvShowSeasonPoster << ImageType::TvShowSeasonThumb;
}

void TvShow::setDir(const QString &dir)
{
    m_dir = dir;
}

QString TvShow::status() const
{
    return m_status;
}

void TvShow::setStatus(const QString &status)
{
    m_status = status;
    setChanged(true);
}

int TvShow::votes() const
{
    return m_votes;
}

void TvShow::setVotes(int votes)
{
    m_votes = votes;
    setChanged(true);
}

int TvShow::top250() const
{
    return m_top250;
}

void TvShow::setTop250(int top250)
{
    m_top250 = top250;
    setChanged(true);
}

bool TvShow::hasImage(int type)
{
    return m_hasImage.value(type, false);
}

int TvShow::runtime() const
{
    return m_runtime;
}

void TvShow::setRuntime(int runtime)
{
    m_runtime = runtime;
    setChanged(true);
}

QString TvShow::sortTitle() const
{
    return m_sortTitle;
}

void TvShow::setSortTitle(QString sortTitle)
{
    m_sortTitle = sortTitle;
    setChanged(true);
}

bool TvShow::isDummySeason(int season) const
{
    foreach (TvShowEpisode *episode, m_episodes) {
        if (episode->season() == season && !episode->isDummy())
            return false;
    }
    return true;
}

bool TvShow::hasDummyEpisodes(int season) const
{
    foreach (TvShowEpisode *episode, m_episodes) {
        if (episode->season() == season && episode->isDummy())
            return true;
    }
    return false;
}

bool TvShow::hasDummyEpisodes() const
{
    foreach (TvShowEpisode *episode, m_episodes) {
        if (episode->isDummy())
            return true;
    }
    return false;
}

void TvShow::setShowMissingEpisodes(bool showMissing, bool updateDatabase)
{
    m_showMissingEpisodes = showMissing;
    if (updateDatabase)
        Manager::instance()->database()->setShowMissingEpisodes(this, showMissing);
}

bool TvShow::showMissingEpisodes() const
{
    return m_showMissingEpisodes;
}

void TvShow::setHideSpecialsInMissingEpisodes(bool hideSpecials, bool updateDatabase)
{
    m_hideSpecialsInMissingEpisodes = hideSpecials;
    if (updateDatabase)
        Manager::instance()->database()->setHideSpecialsInMissingEpisodes(this, hideSpecials);
}

bool TvShow::hideSpecialsInMissingEpisodes() const
{
    return m_hideSpecialsInMissingEpisodes;
}

void TvShow::fillMissingEpisodes()
{
    QList<TvShowEpisode*> episodes = Manager::instance()->database()->showsEpisodes(this);
    foreach (TvShowEpisode *episode, episodes) {
        bool found = false;
        for (int i=0, n=m_episodes.count() ; i<n ; ++i) {
            if (m_episodes[i]->season() == episode->season() && m_episodes[i]->episode() == episode->episode()) {
                found = true;
                break;
            }
        }
        if (found) {
            episode->deleteLater();
            continue;
        }

        if (episode->season() == 0 && hideSpecialsInMissingEpisodes()) {
            episode->deleteLater();
            continue;
        }

        episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), false);
        episode->setIsDummy(true);
        episode->setInfosLoaded(true);
        addEpisode(episode);

        bool newSeason = true;
        foreach (TvShowEpisode *existEpisode, this->episodes()) {
            if (existEpisode->season() == episode->season() && existEpisode != episode) {
                newSeason = false;
                break;
            }
        }

        if (newSeason) {
            modelItem()->appendChild(episode->season(), episode->seasonString(), this)->appendChild(episode);
        } else {
            for (int i=0, n=modelItem()->childCount() ; i<n ; ++i) {
                TvShowModelItem *item = modelItem()->child(i);
                if (item->type() == TypeSeason && item->season() == episode->seasonString()) {
                    item->appendChild(episode);
                    break;
                }
            }
        }
    }

    TvShowFilesWidget::instance()->renewModel(true);
}

void TvShow::clearMissingEpisodes()
{
    for (int i=0 ; i<modelItem()->childCount() ; ++i) {
        TvShowModelItem *seasonItem = modelItem()->child(i);
        if (seasonItem->type() != TypeSeason)
            continue;
        bool isDummySeason = true;
        for (int x=0 ; x<seasonItem->childCount() ; ++x) {
            TvShowModelItem *item = seasonItem->child(x);
            if (item->type() != TypeEpisode)
                continue;
            if (item->tvShowEpisode()->isDummy()) {
                seasonItem->removeChildren(x, 1);
                m_episodes.removeOne(item->tvShowEpisode());
                item->tvShowEpisode()->deleteLater();
                x--;
            } else {
                isDummySeason = false;
            }
        }

        if (isDummySeason) {
            modelItem()->removeChildren(i, 1);
            i--;
        }
    }

    TvShowFilesWidget::instance()->renewModel(true);
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
