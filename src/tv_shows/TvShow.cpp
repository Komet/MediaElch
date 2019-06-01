#include "TvShow.h"
#include "globals/Globals.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <algorithm>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/NameFormatter.h"
#include "media_centers/MediaCenterInterface.h"
#include "scrapers/tv_show/TheTvDb.h"
#include "scrapers/tv_show/TvScraperInterface.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/SeasonModelItem.h"
#include "tv_shows/model/TvShowModelItem.h"

using namespace std::chrono_literals;

/**
 * @brief TvShow::TvShow
 * @param dir
 * @param parent
 */
TvShow::TvShow(QString dir, QObject* parent) :
    QObject(parent),
    m_dir{dir},
    m_runtime{0min},
    m_hasTune{false},
    m_downloadsInProgress{false},
    m_infoLoaded{false},
    m_infoFromNfoLoaded{false},
    m_hasChanged{false},
    m_databaseId{-1},
    m_syncNeeded{false},
    m_showMissingEpisodes{false},
    m_hideSpecialsInMissingEpisodes{false}

{
    clear();
    static int m_idCounter = 0;
    m_showId = ++m_idCounter;
}

/**
 * @brief Clears all data
 */
void TvShow::clear()
{
    QVector<TvShowScraperInfos> infos;
    infos << TvShowScraperInfos::Actors        //
          << TvShowScraperInfos::Banner        //
          << TvShowScraperInfos::Certification //
          << TvShowScraperInfos::Fanart        //
          << TvShowScraperInfos::FirstAired    //
          << TvShowScraperInfos::Genres        //
          << TvShowScraperInfos::Network       //
          << TvShowScraperInfos::Overview      //
          << TvShowScraperInfos::Poster        //
          << TvShowScraperInfos::Rating        //
          << TvShowScraperInfos::SeasonPoster  //
          << TvShowScraperInfos::Title         //
          << TvShowScraperInfos::Tags          //
          << TvShowScraperInfos::ExtraArts     //
          << TvShowScraperInfos::ExtraFanarts  //
          << TvShowScraperInfos::Thumb         //
          << TvShowScraperInfos::SeasonThumb   //
          << TvShowScraperInfos::Runtime;
    clear(infos);
    m_nfoContent.clear();
}

void TvShow::clear(QVector<TvShowScraperInfos> infos)
{
    if (infos.contains(TvShowScraperInfos::Actors)) {
        m_actors.clear();
    }
    if (infos.contains(TvShowScraperInfos::Banner)) {
        m_banners.clear();
        m_imagesToRemove.remove(ImageType::TvShowBanner);
        m_images.insert(ImageType::TvShowBanner, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowBanner, false);
    }
    if (infos.contains(TvShowScraperInfos::Certification)) {
        m_certification = Certification::NoCertification;
    }
    if (infos.contains(TvShowScraperInfos::FirstAired)) {
        m_firstAired = QDate(2000, 02, 30); // invalid date
    }
    if (infos.contains(TvShowScraperInfos::Genres)) {
        m_genres.clear();
    }
    if (infos.contains(TvShowScraperInfos::Network)) {
        m_network.clear();
    }
    if (infos.contains(TvShowScraperInfos::Overview)) {
        m_overview.clear();
    }
    if (infos.contains(TvShowScraperInfos::Poster)) {
        m_posters.clear();
        m_imagesToRemove.remove(ImageType::TvShowPoster);
        m_images.insert(ImageType::TvShowPoster, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowPoster, false);
    }
    if (infos.contains(TvShowScraperInfos::Rating)) {
        m_rating = Rating();
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
    if (infos.contains(TvShowScraperInfos::Title)) {
        m_showTitle.clear();
    }
    if (infos.contains(TvShowScraperInfos::Tags)) {
        m_tags.clear();
    }
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
    if (infos.contains(TvShowScraperInfos::Runtime)) {
        m_runtime = 0min;
    }
    m_hasChanged = false;
}

void TvShow::clearSeasonImageType(ImageType imageType)
{
    QMapIterator<SeasonNumber, QMap<ImageType, QByteArray>> it(m_seasonImages);
    while (it.hasNext()) {
        it.next();
        m_seasonImages[it.key()].insert(imageType, QByteArray());
    }
    QMapIterator<SeasonNumber, QMap<ImageType, bool>> itC(m_hasSeasonImageChanged);
    while (itC.hasNext()) {
        itC.next();
        m_hasSeasonImageChanged[itC.key()].insert(imageType, false);
    }
}

/**
 * @brief Adds an episode
 * @param episode Episode to add
 */
void TvShow::addEpisode(TvShowEpisode* episode)
{
    m_episodes.push_back(episode);
}

/**
 * @brief TvShow::episodeCount
 * @return Number of child episodes
 */
int TvShow::episodeCount() const
{
    return m_episodes.size();
}

/**
 * @brief Load data using the given MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use
 * @return Loading was successful or not
 */
bool TvShow::loadData(MediaCenterInterface* mediaCenterInterface, bool reloadFromNfo)
{
    if (hasChanged() || (m_infoLoaded && m_infoFromNfoLoaded && !reloadFromNfo)) {
        return m_infoLoaded;
    }

    const bool infoLoaded = [&]() {
        return reloadFromNfo ? mediaCenterInterface->loadTvShow(this)
                             : mediaCenterInterface->loadTvShow(this, nfoContent());
    }();

    if (!infoLoaded) {
        QStringList dirParts = this->dir().split(QDir::separator());
        setName(NameFormatter::instance()->formatName(dirParts.last()));
    }
    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    setChanged(false);

    m_hasImage.clear();
    m_hasImage.insert(ImageType::TvShowExtraFanart, !mediaCenterInterface->extraFanartNames(this).isEmpty());
    for (const auto imageType : TvShow::imageTypes()) {
        m_hasImage.insert(imageType, !mediaCenterInterface->imageFileName(this, imageType).isEmpty());
    }

    return infoLoaded;
}

/**
 * @brief Loads the shows data using a scraper
 * @param id ID of the show for the given scraper
 * @param tvScraperInterface Scraper to use
 */
void TvShow::loadData(TvDbId id,
    TvScraperInterface* tvScraperInterface,
    TvShowUpdateType type,
    QVector<TvShowScraperInfos> infosToLoad)
{
    if (tvScraperInterface->identifier() == TheTvDb::scraperIdentifier) {
        setTvdbId(id);
    }
    m_infosToLoad = infosToLoad;
    tvScraperInterface->loadTvShowData(id, this, type, infosToLoad);
}

/**
 * @brief Saves the shows data
 * @param mediaCenterInterface MediaCenterInterface to use
 * @return Saving was successful
 */
bool TvShow::saveData(MediaCenterInterface* mediaCenterInterface)
{
    qDebug() << "Entered";
    bool saved = mediaCenterInterface->saveTvShow(this);
    if (!m_infoLoaded) {
        m_infoLoaded = saved;
    }

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
    for (Actor& actor : actors()) {
        actor.image = QByteArray();
    }
    m_extraFanartImagesToAdd.clear();
}

/**
 * @brief TvShow::hasNewEpisodes
 * @return
 */
bool TvShow::hasNewEpisodes() const
{
    const auto checkInfoLoaded = [](TvShowEpisode* episode) { return !episode->infoLoaded(); };
    return std::any_of(m_episodes.cbegin(), m_episodes.cend(), checkInfoLoaded);
}

/**
 * @brief TvShow::hasNewEpisodesInSeason
 * @param season Season number
 * @return
 */
bool TvShow::hasNewEpisodesInSeason(SeasonNumber season) const
{
    return std::any_of(m_episodes.cbegin(), m_episodes.cend(), [season](const TvShowEpisode* const episode) {
        return episode->season() == season && !episode->infoLoaded();
    });
}

/*** GETTER ***/

QVector<TvShowScraperInfos> TvShow::infosToLoad() const
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
double TvShow::rating() const
{
    return m_rating.rating;
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
QVector<QString*> TvShow::genresPointer()
{
    QVector<QString*> genres;
    for (int i = 0, n = m_genres.size(); i < n; ++i) {
        genres.append(&m_genres[i]);
    }
    return genres;
}

/**
 * @property TvShow::certification
 * @brief Certification of the show
 * @return Certification
 * @see TvShow::setCertification
 */
Certification TvShow::certification() const
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
TvDbId TvShow::tvdbId() const
{
    return m_tvdbId;
}

TvDbId TvShow::id() const
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
QVector<Certification> TvShow::certifications() const
{
    QVector<Certification> certifications;
    for (TvShowEpisode* episode : m_episodes) {
        if (!certifications.contains(episode->certification()) && episode->certification().isValid()) {
            certifications.append(episode->certification());
        }
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
QVector<Actor> TvShow::actors() const
{
    return m_actors;
}

/**
 * @property TvShow::posters
 * @brief Tv Show posters
 * @return List of posters
 * @see TvShow::addPoster
 * @see TvShow::setPoster
 * @see TvShow::setPosters
 */
QVector<Poster> TvShow::posters() const
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
QVector<Poster> TvShow::banners() const
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
QVector<Poster> TvShow::backdrops() const
{
    return m_backdrops;
}

/**
 * @brief TvShow::seasonPosters
 * @param season
 * @return
 */
QVector<Poster> TvShow::seasonPosters(SeasonNumber season) const
{
    if (!m_seasonPosters.contains(season)) {
        return QVector<Poster>();
    }
    return m_seasonPosters[season];
}

QVector<Poster> TvShow::seasonBackdrops(SeasonNumber season) const
{
    if (!m_seasonBackdrops.contains(season)) {
        return QVector<Poster>();
    }
    return m_seasonBackdrops[season];
}

QVector<Poster> TvShow::seasonBanners(SeasonNumber season, bool returnAll) const
{
    if (!m_seasonBanners.contains(season) && !returnAll) {
        return QVector<Poster>();
    }
    if (!returnAll) {
        return m_seasonBanners[season];
    }

    QVector<Poster> banners;
    if (m_seasonBanners.contains(season)) {
        banners = m_seasonBanners[season];
    }

    QMapIterator<SeasonNumber, QVector<Poster>> it(m_seasonBanners);
    while (it.hasNext()) {
        it.next();
        if (it.key() == season) {
            continue;
        }
        banners << it.value();
    }
    return banners;
}

QVector<Poster> TvShow::seasonThumbs(SeasonNumber season, bool returnAll) const
{
    if (!m_seasonThumbs.contains(season) && !returnAll) {
        return QVector<Poster>();
    }
    if (!returnAll) {
        return m_seasonThumbs[season];
    }

    QVector<Poster> thumbs;
    if (m_seasonThumbs.contains(season)) {
        thumbs = m_seasonThumbs[season];
    }

    QMapIterator<SeasonNumber, QVector<Poster>> it(m_seasonThumbs);
    while (it.hasNext()) {
        it.next();
        if (it.key() == season) {
            continue;
        }
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
TvShowEpisode* TvShow::episode(SeasonNumber season, EpisodeNumber episode)
{
    for (int i = 0, n = m_episodes.count(); i < n; ++i) {
        if (m_episodes[i]->season() == season && m_episodes[i]->episode() == episode) {
            return m_episodes[i];
        }
    }
    return new TvShowEpisode(QStringList(), this);
}

/**
 * @brief TvShow::seasons
 * @return
 */
QVector<SeasonNumber> TvShow::seasons(bool includeDummies) const
{
    QVector<SeasonNumber> seasons;
    for (TvShowEpisode* episode : m_episodes) {
        if (episode->isDummy() && !includeDummies) {
            continue;
        }
        if (!seasons.contains(episode->season()) && episode->season() != SeasonNumber::NoSeason) {
            seasons.append(episode->season());
        }
    }
    return seasons;
}

const QVector<TvShowEpisode*>& TvShow::episodes() const
{
    return m_episodes;
}

QVector<TvShowEpisode*> TvShow::episodes(SeasonNumber season) const
{
    QVector<TvShowEpisode*> episodes;
    for (TvShowEpisode* episode : m_episodes) {
        if (episode->season() == season) {
            episodes.push_back(episode);
        }
    }

    return episodes;
}

/**
 * @brief TvShow::modelItem
 * @return
 */
TvShowModelItem* TvShow::modelItem()
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
void TvShow::setRating(double rating)
{
    m_rating.rating = rating;
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
    for (const QString& genre : genres) {
        if (!genre.isEmpty()) {
            m_genres.append(genre);
        }
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
    if (genre.isEmpty()) {
        return;
    }
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
void TvShow::setCertification(Certification certification)
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
void TvShow::setTvdbId(TvDbId id)
{
    if (m_id.isValid()) {
        m_id = id;
    }
    m_tvdbId = id;
    setChanged(true);
}

void TvShow::setId(TvDbId id)
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
void TvShow::setPosters(QVector<Poster> posters)
{
    m_posters = posters;
    setChanged(true);
}

/**
 * @brief Sets all banners
 * @param banners
 * @see TvShow::banners
 */
void TvShow::setBanners(QVector<Poster> banners)
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
    if (m_posters.size() < index) {
        return;
    }
    m_posters[index] = poster;
    setChanged(true);
}

/**
 * @brief Appends a list of backdrops
 * @param backdrops
 * @see TvShow::backdrops
 */
void TvShow::setBackdrops(QVector<Poster> backdrops)
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
    if (m_backdrops.size() < index) {
        return;
    }
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
void TvShow::addSeasonPoster(SeasonNumber season, Poster poster)
{
    if (!m_seasonPosters.contains(season)) {
        QVector<Poster> posters;
        m_seasonPosters.insert(season, posters);
    }

    m_seasonPosters[season].append(poster);
    setChanged(true);
}

void TvShow::addSeasonBackdrop(SeasonNumber season, Poster poster)
{
    if (!m_seasonBackdrops.contains(season)) {
        QVector<Poster> posters;
        m_seasonBackdrops.insert(season, posters);
    }

    m_seasonBackdrops[season].append(poster);
    setChanged(true);
}

void TvShow::addSeasonBanner(SeasonNumber season, Poster poster)
{
    if (!m_seasonBanners.contains(season)) {
        QVector<Poster> posters;
        m_seasonBanners.insert(season, posters);
    }

    m_seasonBanners[season].append(poster);
    setChanged(true);
}

void TvShow::addSeasonThumb(SeasonNumber season, Poster poster)
{
    if (!m_seasonThumbs.contains(season)) {
        QVector<Poster> posters;
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
void TvShow::setModelItem(TvShowModelItem* item)
{
    if (item == nullptr) {
        qCritical() << "[TvShow] Tried to set nullptr model item";
        return;
    }
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
void TvShow::removeActor(Actor* actor)
{
    for (int i = 0, n = m_actors.size(); i < n; ++i) {
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

QVector<ExtraFanart> TvShow::extraFanarts(MediaCenterInterface* mediaCenterInterface)
{
    if (m_extraFanarts.isEmpty()) {
        m_extraFanarts = mediaCenterInterface->extraFanartNames(this);
    }
    for (const auto& file : m_extraFanartsToRemove) {
        m_extraFanarts.removeOne(file);
    }
    QVector<ExtraFanart> fanarts;
    for (const auto& file : m_extraFanarts) {
        ExtraFanart f;
        f.path = file;
        fanarts.append(f);
    }
    for (const auto& img : m_extraFanartImagesToAdd) {
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

QVector<QByteArray> TvShow::extraFanartImagesToAdd()
{
    return m_extraFanartImagesToAdd;
}

void TvShow::clearExtraFanartData()
{
    m_extraFanartImagesToAdd.clear();
    m_extraFanartsToRemove.clear();
    m_extraFanarts.clear();
}

QMap<ImageType, QVector<SeasonNumber>> TvShow::imagesToRemove() const
{
    return m_imagesToRemove;
}

void TvShow::removeImage(ImageType type, SeasonNumber season)
{
    if (TvShow::seasonImageTypes().contains(type)) {
        if (m_seasonImages.contains(season) && !m_seasonImages.value(season).value(type, QByteArray()).isNull()) {
            m_seasonImages[season].insert(type, QByteArray());
            if (!m_hasSeasonImageChanged.contains(season)) {
                m_hasSeasonImageChanged.insert(season, QMap<ImageType, bool>());
            }
            m_hasSeasonImageChanged[season].insert(type, false);
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type, QVector<SeasonNumber>{season});
        } else if (m_imagesToRemove.contains(type) && !m_imagesToRemove.value(type).contains(season)) {
            m_imagesToRemove[type].append(season);
        }
    } else {
        if (!m_images.value(type, QByteArray()).isNull()) {
            m_images.insert(type, QByteArray());
            m_hasImageChanged.insert(type, false);
        } else {
            m_imagesToRemove.insert(type, QVector<SeasonNumber>{SeasonNumber::NoSeason});
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

bool TvShow::lessThan(TvShow* a, TvShow* b)
{
    return (QString::localeAwareCompare(Helper::appendArticle(a->name()), Helper::appendArticle(b->name())) < 0);
}

QByteArray TvShow::image(ImageType imageType)
{
    return m_images.value(imageType);
}

QByteArray TvShow::seasonImage(SeasonNumber season, ImageType imageType)
{
    if (m_seasonImages.contains(season)) {
        return m_seasonImages.value(season).value(imageType, QByteArray());
    }
    return QByteArray();
}

void TvShow::setImage(ImageType imageType, QByteArray image)
{
    m_images.insert(imageType, image);
    m_hasImageChanged.insert(imageType, true);
    setChanged(true);
}

void TvShow::setSeasonImage(SeasonNumber season, ImageType imageType, QByteArray image)
{
    if (!m_seasonImages.contains(season)) {
        m_seasonImages.insert(season, QMap<ImageType, QByteArray>());
    }
    m_seasonImages[season].insert(imageType, image);

    if (!m_hasSeasonImageChanged.contains(season)) {
        m_hasSeasonImageChanged.insert(season, QMap<ImageType, bool>());
    }
    m_hasSeasonImageChanged[season].insert(imageType, true);
    setChanged(true);
}

bool TvShow::imageHasChanged(ImageType imageType) const
{
    return m_hasImageChanged.value(imageType, false);
}

bool TvShow::seasonImageHasChanged(SeasonNumber season, ImageType imageType) const
{
    if (m_hasSeasonImageChanged.contains(season)) {
        return m_hasSeasonImageChanged.value(season).value(imageType, false);
    }
    return false;
}

QVector<ImageType> TvShow::imageTypes()
{
    return {ImageType::TvShowPoster,
        ImageType::TvShowCharacterArt,
        ImageType::TvShowClearArt,
        ImageType::TvShowLogos,
        ImageType::TvShowBackdrop,
        ImageType::TvShowBanner,
        ImageType::TvShowThumb};
}

QVector<ImageType> TvShow::seasonImageTypes()
{
    return {ImageType::TvShowSeasonBackdrop,
        ImageType::TvShowSeasonBanner,
        ImageType::TvShowSeasonPoster,
        ImageType::TvShowSeasonThumb};
}

void TvShow::setDir(const QString& dir)
{
    m_dir = dir;
}

QString TvShow::status() const
{
    return m_status;
}

void TvShow::setStatus(const QString& status)
{
    m_status = status;
    setChanged(true);
}

int TvShow::votes() const
{
    return m_rating.voteCount;
}

void TvShow::setVotes(int votes)
{
    m_rating.voteCount = votes;
    setChanged(true);
}

int TvShow::top250() const
{
    return m_imdbTop250;
}

void TvShow::setTop250(int top250)
{
    m_imdbTop250 = top250;
    setChanged(true);
}

bool TvShow::hasImage(ImageType type)
{
    return m_hasImage.value(type, false);
}

std::chrono::minutes TvShow::runtime() const
{
    return m_runtime;
}

void TvShow::setRuntime(std::chrono::minutes runtime)
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

bool TvShow::isDummySeason(SeasonNumber season) const
{
    for (TvShowEpisode* episode : m_episodes) {
        if (episode->season() == season && !episode->isDummy()) {
            return false;
        }
    }
    return true;
}

bool TvShow::hasDummyEpisodes(SeasonNumber season) const
{
    return std::any_of(m_episodes.cbegin(), m_episodes.cend(), [season](const TvShowEpisode* const episode) {
        return episode->season() == season && episode->isDummy();
    });
}

bool TvShow::hasDummyEpisodes() const
{
    return std::any_of(
        m_episodes.cbegin(), m_episodes.cend(), [](const TvShowEpisode* const episode) { return episode->isDummy(); });
}

void TvShow::setShowMissingEpisodes(bool showMissing, bool updateDatabase)
{
    m_showMissingEpisodes = showMissing;
    if (updateDatabase) {
        Manager::instance()->database()->setShowMissingEpisodes(this, showMissing);
    }
}

bool TvShow::showMissingEpisodes() const
{
    return m_showMissingEpisodes;
}

void TvShow::setHideSpecialsInMissingEpisodes(bool hideSpecials, bool updateDatabase)
{
    m_hideSpecialsInMissingEpisodes = hideSpecials;
    if (updateDatabase) {
        Manager::instance()->database()->setHideSpecialsInMissingEpisodes(this, hideSpecials);
    }
}

bool TvShow::hideSpecialsInMissingEpisodes() const
{
    return m_hideSpecialsInMissingEpisodes;
}

void TvShow::fillMissingEpisodes()
{
    QVector<TvShowEpisode*> episodes = Manager::instance()->database()->showsEpisodes(this);
    for (TvShowEpisode* episode : episodes) {
        if (episode == nullptr) {
            qCritical() << "[TvShow] Episode loaded from database is a nullptr";
            continue;
        }
        bool found = false;
        for (int i = 0, n = m_episodes.count(); i < n; ++i) {
            if (m_episodes[i]->season() == episode->season() && m_episodes[i]->episode() == episode->episode()) {
                found = true;
                break;
            }
        }
        if (found) {
            episode->deleteLater();
            continue;
        }

        if (episode->season() == SeasonNumber::SpecialsSeason && hideSpecialsInMissingEpisodes()) {
            episode->deleteLater();
            continue;
        }

        episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), false);
        episode->setIsDummy(true);
        episode->setInfosLoaded(true);
        addEpisode(episode);

        bool newSeason = true;
        for (const TvShowEpisode* existEpisode : m_episodes) {
            if (existEpisode->season() == episode->season() && existEpisode != episode) {
                newSeason = false;
                break;
            }
        }

        if (newSeason) {
            modelItem()->appendSeason(episode->season(), episode->seasonString(), this)->appendEpisode(episode);

        } else {
            for (int i = 0, n = modelItem()->seasons().size(); i < n; ++i) {
                SeasonModelItem* item = modelItem()->seasonAtIndex(i);
                if (item->type() == TvShowType::Season && item->season() == episode->seasonString()) {
                    item->appendEpisode(episode);
                    break;
                }
            }
        }
    }

    TvShowFilesWidget::instance().renewModel(true);
}

void TvShow::clearMissingEpisodes()
{
    for (int i = 0; i < modelItem()->seasons().size(); ++i) {
        SeasonModelItem* seasonItem = modelItem()->seasonAtIndex(i);
        if (seasonItem == nullptr) {
            qCritical() << "[TvShow] (Season) item is a nullptr";
            continue;
        }
        if (seasonItem->type() != TvShowType::Season) {
            continue;
        }
        bool isDummySeason = true;
        for (int x = 0; x < seasonItem->episodes().size(); ++x) {
            EpisodeModelItem* episodeItem = seasonItem->episodeAtIndex(x);
            if (episodeItem == nullptr) {
                qCritical() << "[TvShow] (Episode) item is a nullptr";
                continue;
            }
            if (episodeItem->type() != TvShowType::Episode) {
                continue;
            }
            if (episodeItem->tvShowEpisode()->isDummy()) {
                m_episodes.removeOne(episodeItem->tvShowEpisode());
                episodeItem->tvShowEpisode()->deleteLater();
                seasonItem->removeChildren(x, 1);
                x--;
            } else {
                isDummySeason = false;
            }
        }

        if (isDummySeason) {
            modelItem()->removeChildren(i, 1);
            --i;
        }
    }

    TvShowFilesWidget::instance().renewModel(true);
}

/*** DEBUG ***/

QDebug operator<<(QDebug dbg, const TvShow& show)
{
    QString nl = "\n";
    QString out;
    out.append("TvShow").append(nl);
    out.append(QStringLiteral("  Dir:           ").append(show.dir()).append(nl));
    out.append(QStringLiteral("  Name:          ").append(show.name()).append(nl));
    out.append(QStringLiteral("  ShowTitle:     ").append(show.showTitle()).append(nl));
    out.append(QStringLiteral("  Rating:        %1").arg(show.rating()).append(nl));
    out.append(QStringLiteral("  FirstAired:    ").append(show.firstAired().toString("yyyy-MM-dd")).append(nl));
    out.append(QStringLiteral("  Certification: ").append(show.certification().toString()).append(nl));
    out.append(QStringLiteral("  Network:       ").append(show.network()).append(nl));
    out.append(QStringLiteral("  Overview:      ").append(show.overview())).append(nl);
    out.append(QStringLiteral("  Status:        ").append(show.status())).append(nl);
    for (const QString& genre : show.genres()) {
        out.append(QString("  Genre:         ").append(genre)).append(nl);
    }
    for (const Actor& actor : show.actors()) {
        out.append(QStringLiteral("  Actor:         ").append(nl));
        out.append(QStringLiteral("    Name:  ").append(actor.name)).append(nl);
        out.append(QStringLiteral("    Role:  ").append(actor.role)).append(nl);
        out.append(QStringLiteral("    Thumb: ").append(actor.thumb)).append(nl);
    }
    /*
    for (const QString &studio: movie.studios())
        out.append(QString("  Studio:         ").append(studio)).append(nl);
    for (const QString &country: movie.countries())
        out.append(QString("  Country:       ").append(country)).append(nl);
    for (const Poster &poster: movie.posters()) {
        out.append(QString("  Poster:       ")).append(nl);
        out.append(QString("    ID:       ").append(poster.id)).append(nl);
        out.append(QString("    Original: ").append(poster.originalUrl.toString())).append(nl);
        out.append(QString("    Thumb:    ").append(poster.thumbUrl.toString())).append(nl);
    }
    for (const Poster &backdrop: movie.backdrops()) {
        out.append(QString("  Backdrop:       ")).append(nl);
        out.append(QString("    ID:       ").append(backdrop.id)).append(nl);
        out.append(QString("    Original: ").append(backdrop.originalUrl.toString())).append(nl);
        out.append(QString("    Thumb:    ").append(backdrop.thumbUrl.toString())).append(nl);
    }
    */
    dbg.nospace() << out;
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const TvShow* show)
{
    dbg.nospace() << *show;
    return dbg.space();
}
