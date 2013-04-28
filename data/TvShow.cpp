#include "TvShow.h"
#include "globals/Globals.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include "globals/Globals.h"
#include "globals/Helper.h"
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
    m_backdropImageChanged = false;
    m_posterImageChanged = false;
    m_bannerImageChanged = false;
    m_logoImageChanged = false;
    m_clearArtImageChanged = false;
    m_characterArtImageChanged = false;
    m_thumbImageChanged = false;
    m_hasChanged = false;
    clear();
    m_downloadsInProgress = false;
    static int m_idCounter = 0;
    m_showId = ++m_idCounter;
    m_databaseId = -1;
    m_syncNeeded = false;
    m_hasTune = false;
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
          << TvShowScraperInfos::SeasonThumb;
    clear(infos);
    m_nfoContent.clear();
}

void TvShow::clear(QList<int> infos)
{
    if (infos.contains(TvShowScraperInfos::Actors))
        m_actors.clear();
    if (infos.contains(TvShowScraperInfos::Banner)) {
        m_banners.clear();
        m_bannerImageChanged = false;
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
        m_posterImageChanged = false;
    }
    if (infos.contains(TvShowScraperInfos::Rating))
        m_rating = 0;
    if (infos.contains(TvShowScraperInfos::SeasonPoster)) {
        m_seasonPosters.clear();
        m_seasonPosterImages.clear();
        m_seasonPosterImagesChanged.clear();
        m_imagesToRemove.remove(TypeSeasonPoster);
    }
    if (infos.contains(TvShowScraperInfos::SeasonBackdrop)) {
        m_seasonBackdrops.clear();
        m_seasonBackdropImages.clear();
        m_seasonBackdropImagesChanged.clear();
        m_imagesToRemove.remove(TypeSeasonBackdrop);
    }
    if (infos.contains(TvShowScraperInfos::SeasonBanner)) {
        m_seasonBanners.clear();
        m_seasonBannerImages.clear();
        m_seasonBannerImagesChanged.clear();
        m_imagesToRemove.remove(TypeSeasonBanner);
    }
    if (infos.contains(TvShowScraperInfos::SeasonThumb)) {
        m_seasonThumbs.clear();
        m_seasonThumbImages.clear();
        m_seasonThumbImagesChanged.clear();
        m_imagesToRemove.remove(TypeSeasonThumb);
    }
    if (infos.contains(TvShowScraperInfos::Title))
        m_showTitle.clear();
    if (infos.contains(TvShowScraperInfos::Tags))
        m_tags.clear();
    if (infos.contains(TvShowScraperInfos::Fanart)) {
        m_backdrops.clear();
        m_backdropImageChanged = false;
    }
    if (infos.contains(TvShowScraperInfos::ExtraArts)) {
        m_logoImage = QByteArray();
        m_logoImageChanged = false;
        m_thumbImage = QByteArray();
        m_thumbImageChanged = false;
        m_clearArtImage = QByteArray();
        m_clearArtImageChanged = false;
        m_characterArtImage = QByteArray();
        m_characterArtImageChanged = false;
        m_imagesToRemove.remove(TypeLogo);
        m_imagesToRemove.remove(TypeClearArt);
        m_imagesToRemove.remove(TypeCdArt);
        m_imagesToRemove.remove(TypeThumb);
    }
    if (infos.contains(TvShowScraperInfos::ExtraFanarts)) {
        m_extraFanartsToRemove.clear();
        m_extraFanartImagesToAdd.clear();
        m_extraFanarts.clear();
    }
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
bool TvShow::loadData(MediaCenterInterface *mediaCenterInterface, bool reloadFromNfo)
{
    if (hasChanged() || (m_infoLoaded && m_infoFromNfoLoaded))
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
    return infoLoaded;
}

/**
 * @brief Loads the shows data using a scraper
 * @param id ID of the show for the given scraper
 * @param tvScraperInterface Scraper to use
 */
void TvShow::loadData(QString id, TvScraperInterface *tvScraperInterface, TvShowUpdateType type, QList<int> infosToLoad)
{
    if (tvScraperInterface->name() == "The TV DB")
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
    m_posterImage = QByteArray();
    m_backdropImage = QByteArray();
    m_bannerImage = QByteArray();
    m_logoImage = QByteArray();
    m_thumbImage = QByteArray();
    m_clearArtImage = QByteArray();
    m_characterArtImage = QByteArray();
    foreach (int season, seasons()) {
        m_seasonPosterImages[season] = QByteArray();
        m_seasonBackdropImages[season] = QByteArray();
        m_seasonBannerImages[season] = QByteArray();
        m_seasonThumbImages[season] = QByteArray();
    }
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
 * @brief TvShow::posterImage
 * @return
 */
QByteArray TvShow::posterImage()
{
    return m_posterImage;
}

/**
 * @brief TvShow::backdropImage
 * @return
 */
QByteArray TvShow::backdropImage()
{
    return m_backdropImage;
}

/**
 * @brief TvShow::bannerImage
 * @return
 */
QByteArray TvShow::bannerImage()
{
    return m_bannerImage;
}

/**
 * @brief TvShow::logoImage
 * @return
 */
QByteArray TvShow::logoImage()
{
    return m_logoImage;
}

QByteArray TvShow::thumbImage()
{
    return m_thumbImage;
}

/**
 * @brief TvShow::clearArtImage
 * @return
 */
QByteArray TvShow::clearArtImage()
{
    return m_clearArtImage;
}

/**
 * @brief TvShow::characterArtImage
 * @return
 */
QByteArray TvShow::characterArtImage()
{
    return m_characterArtImage;
}

/**
 * @brief TvShow::seasonPosterImage
 * @param season
 * @return
 */
QByteArray TvShow::seasonPosterImage(int season)
{
    if (!m_seasonPosterImages.contains(season))
        m_seasonPosterImages.insert(season, QByteArray());

    return m_seasonPosterImages[season];
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

QByteArray TvShow::seasonBackdropImage(int season)
{
    if (!m_seasonBackdropImages.contains(season))
        m_seasonBackdropImages.insert(season, QByteArray());

    return m_seasonBackdropImages[season];
}

QList<Poster> TvShow::seasonBackdrops(int season) const
{
    if (!m_seasonBackdrops.contains(season))
        return QList<Poster>();

    return m_seasonBackdrops[season];
}

QByteArray TvShow::seasonThumbImage(int season)
{
    if (!m_seasonThumbImages.contains(season))
        m_seasonThumbImages.insert(season, QByteArray());

    return m_seasonThumbImages[season];
}

QByteArray TvShow::seasonBannerImage(int season)
{
    if (!m_seasonBannerImages.contains(season))
        m_seasonBannerImages.insert(season, QByteArray());

    return m_seasonBannerImages[season];
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

bool TvShow::thumbImageChanged() const
{
    return m_thumbImageChanged;
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

bool TvShow::seasonBackdropImageChanged(int season) const
{
    return m_seasonBackdropImagesChanged.contains(season);
}

bool TvShow::seasonBannerImageChanged(int season) const
{
    return m_seasonBannerImagesChanged.contains(season);
}

bool TvShow::seasonThumbImageChanged(int season) const
{
    return m_seasonThumbImagesChanged.contains(season);
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
void TvShow::setPosterImage(QByteArray poster)
{
    m_posterImage = poster;
    m_posterImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the banner image
 * @param banner
 */
void TvShow::setBannerImage(QByteArray banner)
{
    m_bannerImage = banner;
    m_bannerImageChanged = true;
    setChanged(true);
}

void TvShow::setThumbImage(QByteArray thumb)
{
    m_thumbImage = thumb;
    m_thumbImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the logo image
 * @param img
 */
void TvShow::setLogoImage(QByteArray img)
{
    m_logoImage = img;
    m_logoImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the clear art image
 * @param img
 */
void TvShow::setClearArtImage(QByteArray img)
{
    m_clearArtImage = img;
    m_clearArtImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the character art image
 * @param img
 */
void TvShow::setCharacterArtImage(QByteArray img)
{
    m_characterArtImage = img;
    m_characterArtImageChanged = true;
    setChanged(true);
}

/**
 * @brief Sets the backdrop image
 * @param backdrop
 */
void TvShow::setBackdropImage(QByteArray backdrop)
{
    m_backdropImage = backdrop;
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
void TvShow::setSeasonPosterImage(int season, QByteArray poster)
{
    if (m_seasonPosterImages.contains(season))
        m_seasonPosterImages[season] = poster;
    else
        m_seasonPosterImages.insert(season, poster);

    if (!m_seasonPosterImagesChanged.contains(season))
        m_seasonPosterImagesChanged.append(season);
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

void TvShow::setSeasonBackdropImage(int season, QByteArray poster)
{
    if (m_seasonBackdropImages.contains(season))
        m_seasonBackdropImages[season] = poster;
    else
        m_seasonBackdropImages.insert(season, poster);

    if (!m_seasonBackdropImagesChanged.contains(season))
        m_seasonBackdropImagesChanged.append(season);
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

void TvShow::setSeasonBannerImage(int season, QByteArray poster)
{
    if (m_seasonBannerImages.contains(season))
        m_seasonBannerImages[season] = poster;
    else
        m_seasonBannerImages.insert(season, poster);

    if (!m_seasonBannerImagesChanged.contains(season))
        m_seasonBannerImagesChanged.append(season);
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

void TvShow::setSeasonThumbImage(int season, QByteArray poster)
{
    if (m_seasonThumbImages.contains(season))
        m_seasonThumbImages[season] = poster;
    else
        m_seasonThumbImages.insert(season, poster);

    if (!m_seasonThumbImagesChanged.contains(season))
        m_seasonThumbImagesChanged.append(season);
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

QMap<ImageType, QList<int> > TvShow::imagesToRemove() const
{
    return m_imagesToRemove;
}

void TvShow::removeImage(ImageType type, int season)
{
    switch (type) {
    case TypePoster:
        if (!m_posterImage.isNull()) {
            m_posterImage = QByteArray();
            m_posterImageChanged = false;
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << -2);
        }
        break;
    case TypeBackdrop:
        if (!m_backdropImage.isNull()) {
            m_backdropImage = QByteArray();
            m_backdropImageChanged = false;
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << -2);
        }
        break;
    case TypeBanner:
        if (!m_bannerImage.isNull()) {
            m_bannerImage = QByteArray();
            m_bannerImageChanged = false;
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << -2);
        }
        break;
    case TypeSeasonPoster:
        if (m_seasonPosterImages.contains(season) && !m_seasonPosterImages[season].isNull()) {
            m_seasonPosterImages[season] = QByteArray();
            m_seasonPosterImagesChanged.removeOne(season);
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << season);
        } else if (m_imagesToRemove.contains(type) && !m_imagesToRemove[type].contains(season)) {
            m_imagesToRemove[type].append(season);
        }
        break;
    case TypeSeasonBackdrop:
        if (m_seasonBackdropImages.contains(season) && !m_seasonBackdropImages[season].isNull()) {
            m_seasonBackdropImages[season] = QByteArray();
            m_seasonBackdropImagesChanged.removeOne(season);
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << season);
        } else if (m_imagesToRemove.contains(type) && !m_imagesToRemove[type].contains(season)) {
            m_imagesToRemove[type].append(season);
        }
        break;
    case TypeSeasonBanner:
        if (m_seasonBannerImages.contains(season) && !m_seasonBannerImages[season].isNull()) {
            m_seasonBannerImages[season] = QByteArray();
            m_seasonBannerImagesChanged.removeOne(season);
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << season);
        } else if (m_imagesToRemove.contains(type) && !m_imagesToRemove[type].contains(season)) {
            m_imagesToRemove[type].append(season);
        }
        break;
    case TypeSeasonThumb:
        if (m_seasonThumbImages.contains(season) && !m_seasonThumbImages[season].isNull()) {
            m_seasonThumbImages[season] = QByteArray();
            m_seasonThumbImagesChanged.removeOne(season);
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << season);
        } else if (m_imagesToRemove.contains(type) && !m_imagesToRemove[type].contains(season)) {
            m_imagesToRemove[type].append(season);
        }
        break;
    case TypeThumb:
        if (!m_thumbImage.isNull()) {
            m_thumbImage = QByteArray();
            m_thumbImageChanged = false;
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << -2);
        }
        break;
    case TypeLogo:
        if (!m_logoImage.isNull()) {
            m_logoImage = QByteArray();
            m_logoImageChanged = false;
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << -2);
        }
        break;
    case TypeClearArt:
        if (!m_clearArtImage.isNull()) {
            m_clearArtImage = QByteArray();
            m_clearArtImageChanged = false;
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << -2);
        }
        break;
    case TypeCharacterArt:
        if (!m_characterArtImage.isNull()) {
            m_characterArtImage = QByteArray();
            m_characterArtImageChanged = false;
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QList<int>() << -2);
        }
        break;
    default:
        break;
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
    return (QString::localeAwareCompare(Helper::appendArticle(a->name()), Helper::appendArticle(b->name())) < 0);
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
