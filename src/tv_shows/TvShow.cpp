#include "TvShow.h"
#include "globals/Globals.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <algorithm>
#include <utility>

#include "file/NameFormatter.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "media_centers/MediaCenterInterface.h"
#include "scrapers/tv_show/ShowMerger.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "tv_shows/model/EpisodeModelItem.h"
#include "tv_shows/model/SeasonModelItem.h"
#include "tv_shows/model/TvShowModelItem.h"

using namespace std::chrono_literals;

TvShow::TvShow(mediaelch::DirectoryPath dir, QObject* parent) : QObject(parent), m_dir{std::move(dir)}, m_runtime{0min}
{
    clear();
    static int m_idCounter = 0;
    m_showId = ++m_idCounter;
}

/**
 * \brief Clears all data
 */
void TvShow::clear()
{
    QSet<ShowScraperInfo> infos;
    infos << ShowScraperInfo::Actors        //
          << ShowScraperInfo::Banner        //
          << ShowScraperInfo::Certification //
          << ShowScraperInfo::Fanart        //
          << ShowScraperInfo::FirstAired    //
          << ShowScraperInfo::Genres        //
          << ShowScraperInfo::Network       //
          << ShowScraperInfo::Overview      //
          << ShowScraperInfo::Poster        //
          << ShowScraperInfo::Rating        //
          << ShowScraperInfo::SeasonPoster  //
          << ShowScraperInfo::Title         //
          << ShowScraperInfo::Tags          //
          << ShowScraperInfo::ExtraArts     //
          << ShowScraperInfo::ExtraFanarts  //
          << ShowScraperInfo::Thumb         //
          << ShowScraperInfo::SeasonThumb   //
          << ShowScraperInfo::Runtime;
    clear(infos);
    m_nfoContent.clear();
}

void TvShow::clear(QSet<ShowScraperInfo> infos)
{
    if (infos.contains(ShowScraperInfo::Actors)) {
        m_actors.clear();
    }
    if (infos.contains(ShowScraperInfo::Banner)) {
        m_banners.clear();
        m_imagesToRemove.remove(ImageType::TvShowBanner);
        m_images.insert(ImageType::TvShowBanner, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowBanner, false);
    }
    if (infos.contains(ShowScraperInfo::Certification)) {
        m_certification = Certification::NoCertification;
    }
    if (infos.contains(ShowScraperInfo::FirstAired)) {
        m_firstAired = QDate(2000, 02, 30); // invalid date
    }
    if (infos.contains(ShowScraperInfo::Genres)) {
        m_genres.clear();
    }
    if (infos.contains(ShowScraperInfo::Network)) {
        m_network.clear();
    }
    if (infos.contains(ShowScraperInfo::Overview)) {
        m_overview.clear();
    }
    if (infos.contains(ShowScraperInfo::Poster)) {
        m_posters.clear();
        m_imagesToRemove.remove(ImageType::TvShowPoster);
        m_images.insert(ImageType::TvShowPoster, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowPoster, false);
    }
    if (infos.contains(ShowScraperInfo::Rating)) {
        m_ratings.clear();
    }
    if (infos.contains(ShowScraperInfo::SeasonPoster)) {
        clearSeasonImageType(ImageType::TvShowSeasonPoster);
        m_seasonPosters.clear();
        m_imagesToRemove.remove(ImageType::TvShowSeasonPoster);
    }
    if (infos.contains(ShowScraperInfo::SeasonBackdrop)) {
        clearSeasonImageType(ImageType::TvShowSeasonBackdrop);
        m_seasonBackdrops.clear();
        m_imagesToRemove.remove(ImageType::TvShowSeasonBackdrop);
    }
    if (infos.contains(ShowScraperInfo::SeasonBanner)) {
        clearSeasonImageType(ImageType::TvShowSeasonBanner);
        m_seasonBanners.clear();
        m_imagesToRemove.remove(ImageType::TvShowSeasonBanner);
    }
    if (infos.contains(ShowScraperInfo::SeasonThumb)) {
        clearSeasonImageType(ImageType::TvShowSeasonThumb);
        m_seasonThumbs.clear();
        m_imagesToRemove.remove(ImageType::TvShowSeasonThumb);
    }
    if (infos.contains(ShowScraperInfo::Title)) {
        m_showTitle.clear();
    }
    if (infos.contains(ShowScraperInfo::Tags)) {
        m_tags.clear();
    }
    if (infos.contains(ShowScraperInfo::Fanart)) {
        m_backdrops.clear();
        m_imagesToRemove.remove(ImageType::TvShowBackdrop);
        m_images.insert(ImageType::TvShowBackdrop, QByteArray());
        m_hasImageChanged.insert(ImageType::TvShowBackdrop, false);
    }
    if (infos.contains(ShowScraperInfo::ExtraArts)) {
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
    if (infos.contains(ShowScraperInfo::ExtraFanarts)) {
        m_extraFanartsToRemove.clear();
        m_extraFanartImagesToAdd.clear();
        m_extraFanarts.clear();
    }
    if (infos.contains(ShowScraperInfo::Runtime)) {
        m_runtime = 0min;
    }
    m_hasChanged = false;
}

void TvShow::clearEpisodes(QSet<EpisodeScraperInfo> infos, bool onlyNew)
{
    for (TvShowEpisode* episode : m_episodes) {
        if (!onlyNew || !episode->infoLoaded()) {
            episode->clear(infos);
        }
    }
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
 * \brief Adds an episode
 * \param episode Episode to add
 */
void TvShow::addEpisode(TvShowEpisode* episode)
{
    m_episodes.push_back(episode);
}

/**
 * \brief TvShow::episodeCount
 * \return Number of child episodes
 */
int TvShow::episodeCount() const
{
    return m_episodes.size();
}

/**
 * \brief Load data using the given MediaCenterInterface
 * \param mediaCenterInterface MediaCenterInterface to use
 * \return Loading was successful or not
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
        NameFormatter nameFormatter(Settings::instance()->excludeWords());
        setTitle(nameFormatter.formatName(dir().dirName()));
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
 * \brief Saves the shows data
 * \param mediaCenterInterface MediaCenterInterface to use
 * \return Saving was successful
 */
bool TvShow::saveData(MediaCenterInterface* mediaCenterInterface)
{
    bool saved = mediaCenterInterface->saveTvShow(this);
    if (!m_infoLoaded) {
        m_infoLoaded = saved;
    }

    setSyncNeeded(true);
    setChanged(false);
    clearImages();
    clearExtraFanartData();
    return saved;
}

void TvShow::scrapeData(mediaelch::scraper::TvScraper* scraper,
    const mediaelch::scraper::ShowIdentifier& id,
    const mediaelch::Locale& locale,
    SeasonOrder order,
    TvShowUpdateType updateType,
    const QSet<ShowScraperInfo>& showDetails,
    const QSet<EpisodeScraperInfo>& episodedetails)
{
    using namespace mediaelch;

    // TODO: Remove in future versions.
    m_infosToLoad = showDetails;
    m_episodeInfosToLoad = episodedetails;

    /// Set of seasons that this TV show has. We do not need to load all seasons.
    QSet<SeasonNumber> seasons;
    for (TvShowEpisode* episode : episodes()) {
        seasons << episode->seasonNumber();
    }

    scraper::ShowScrapeJob::Config showScrapeConfig{id, locale, showDetails};
    scraper::SeasonScrapeJob::Config seasonScrapeConfig{id, locale, seasons, order, episodedetails};

    const auto loadEpisodes = [this, updateType, scraper, seasonScrapeConfig, showDetails]() {
        const bool loadNew = isNewEpisodeUpdateType(updateType);
        const auto onEpisodeDone = [this, loadNew, showDetails](scraper::SeasonScrapeJob* job) {
            const auto& scrapedEpisodes = job->episodes();

            for (TvShowEpisode* episode : scrapedEpisodes) {
                // Map according to advanced settings
                const QString network = helper::mapStudio(episode->network());
                const Certification certification = helper::mapCertification(episode->certification());

                episode->setNetwork(network);
                episode->setCertification(certification);
            }

            clearEpisodes(job->config().details, loadNew);
            scraper::copyDetailsToShowEpisodes(*this, scrapedEpisodes, loadNew, job->config().details);

            // Update the TV show's episodes in the database after new details have been merged.
            Database* const database = Manager::instance()->database();
            const int showsSettingsId = database->showsSettingsId(this);
            database->clearEpisodeList(showsSettingsId);
            for (TvShowEpisode* episode : asConst(m_episodes)) {
                database->addEpisodeToShowList(episode, showsSettingsId, episode->tvdbId());
            }
            database->cleanUpEpisodeList(showsSettingsId);

            emit sigLoaded(this, showDetails, job->config().locale);
            job->deleteLater();
        };
        auto* scrapeJob = scraper->loadSeasons(seasonScrapeConfig);
        connect(scrapeJob, &scraper::SeasonScrapeJob::sigFinished, this, onEpisodeDone);
        scrapeJob->execute();
    };

    const auto onShowLoaded = [this, updateType, loadEpisodes](scraper::ShowScrapeJob* job) {
        clear(job->config().details);

        // Map according to advanced settings
        const QStringList genres = helper::mapGenre(job->tvShow().genres());
        const QString network = helper::mapStudio(job->tvShow().network());
        const Certification certification = helper::mapCertification(job->tvShow().certification());

        job->tvShow().setGenres(genres);
        job->tvShow().setNetwork(network);
        job->tvShow().setCertification(certification);

        scraper::copyDetailsToShow(*this, job->tvShow(), job->config().details);
        if (isEpisodeUpdateType(updateType)) {
            loadEpisodes();
        } else {
            emit sigLoaded(this, job->config().details, job->config().locale);
        }
        job->deleteLater();
    };

    if (isShowUpdateType(updateType)) {
        // First load TV show and then episodes.
        auto* scrapeJob = scraper->loadShow(showScrapeConfig);
        connect(scrapeJob, &scraper::ShowScrapeJob::sigFinished, this, onShowLoaded);
        scrapeJob->execute();

    } else if (isEpisodeUpdateType(updateType)) {
        // Only update episodes
        loadEpisodes();
    }
}

/**
 * \brief Clears the movie images to save memory
 */
void TvShow::clearImages()
{
    m_images.clear();
    m_seasonImages.clear();
    m_hasImageChanged.clear();
    m_hasSeasonImageChanged.clear();
    for (auto& actor : m_actors) {
        actor->image = QByteArray();
    }
    m_extraFanartImagesToAdd.clear();
}

bool TvShow::hasNewEpisodes() const
{
    const auto checkInfoLoaded = [](TvShowEpisode* episode) { return !episode->infoLoaded(); };
    return std::any_of(m_episodes.cbegin(), m_episodes.cend(), checkInfoLoaded);
}

/**
 * \brief TvShow::hasNewEpisodesInSeason
 * \param season Season number
 */
bool TvShow::hasNewEpisodesInSeason(SeasonNumber season) const
{
    return std::any_of(m_episodes.cbegin(), m_episodes.cend(), [season](const TvShowEpisode* const episode) {
        return episode->seasonNumber() == season && !episode->infoLoaded();
    });
}

/*** GETTER ***/

bool TvShow::infoLoaded() const
{
    return m_infoLoaded;
}

mediaelch::DirectoryPath TvShow::dir() const
{
    return m_dir;
}

/**
 * \property TvShow::name
 * \brief Name of the show
 * \return Name
 * \see TvShow::setName
 */
QString TvShow::title() const
{
    return m_title;
}

/**
 * \property TvShow::showTitle
 * \brief The title of the show
 * \return Title
 * \see TvShow::setShowTitle
 */
QString TvShow::showTitle() const
{
    return m_showTitle;
}

QString TvShow::originalTitle() const
{
    return m_originalTitle;
}

QString TvShow::sortTitle() const
{
    return m_sortTitle;
}

/**
 * \property TvShow::firstAired
 * \brief First aired date
 * \return Date
 * \see TvShow::setFirstAired
 */
QDate TvShow::firstAired() const
{
    return m_firstAired;
}

/**
 * \property TvShow::genres
 * \brief The genres
 * \return List of genres
 * \see TvShow::addGenre
 * \see TvShow::setGenres
 * \see TvShow::removeGenre
 */
QStringList TvShow::genres() const
{
    return m_genres;
}

/**
 * \brief Constructs a list of pointers to the genres
 * \return List of pointers
 * \see TvShow::genres
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
 * \property TvShow::certification
 * \brief Certification of the show
 * \return Certification
 * \see TvShow::setCertification
 */
Certification TvShow::certification() const
{
    return m_certification;
}

/**
 * \property TvShow::network
 * \brief The network
 * \return Network of the show
 * \see TvShow::setNetwork
 */
QString TvShow::network() const
{
    return m_network;
}

/**
 * \property TvShow::overview
 * \brief The plot
 * \return Plot of the show
 * \see TvShow::setOverview
 */
QString TvShow::overview() const
{
    return m_overview;
}

QVector<Rating>& TvShow::ratings()
{
    return m_ratings;
}

const QVector<Rating>& TvShow::ratings() const
{
    return m_ratings;
}

double TvShow::userRating() const
{
    return m_userRating;
}

TmdbId TvShow::tmdbId() const
{
    return m_tmdbId;
}

TvDbId TvShow::tvdbId() const
{
    return m_tvdbId;
}

ImdbId TvShow::imdbId() const
{
    return m_imdbId;
}

TvMazeId TvShow::tvmazeId() const
{
    return m_tvmazeId;
}

/**
 * \property TvShow::episodeGuideUrl
 * \brief The Episode Guide url of the show
 * \return Episode guide url
 * \see TvShow::setEpisodeGuideUrl
 */
QString TvShow::episodeGuideUrl() const
{
    return m_episodeGuideUrl;
}

/**
 * \brief Constructs a list of all certifications used in child episodes
 * \return List of certifications
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

QVector<const Actor*> TvShow::actors() const
{
    QVector<const Actor*> actorPtrs;
    for (const auto& actor : m_actors) {
        actorPtrs.push_back(actor.get());
    }
    return actorPtrs;
}

QVector<Actor*> TvShow::actors()
{
    QVector<Actor*> actorPtrs;
    for (const auto& actor : m_actors) {
        actorPtrs.push_back(actor.get());
    }
    return actorPtrs;
}

QVector<Poster> TvShow::posters() const
{
    return m_posters;
}

/**
 * \property TvShow::banners
 * \brief Banners of the show
 * \return List of all show banners
 * \see TvShow::setBanners
 * \see TvShow::setBanner
 * \see TvShow::addBanner
 */
QVector<Poster> TvShow::banners() const
{
    return m_banners;
}

/**
 * \property TvShow::backdrops
 * \brief Backdrops of the TV show
 * \return List of backdrops
 * \see TvShow::setBackdrops
 * \see TvShow::addBackdrop
 * \see TvShow::setBackdrop
 */
QVector<Poster> TvShow::backdrops() const
{
    return m_backdrops;
}

QVector<Poster> TvShow::seasonPosters(SeasonNumber season, bool returnAll) const
{
    if (!m_seasonPosters.contains(season) && !returnAll) {
        return QVector<Poster>();
    }
    if (!returnAll) {
        return m_seasonPosters[season];
    }

    QVector<Poster> posters;
    QMapIterator<SeasonNumber, QVector<Poster>> it(m_seasonPosters);
    while (it.hasNext()) {
        it.next();
        posters << it.value();
    }

    return posters;
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

const QMap<SeasonNumber, QVector<Poster>>& TvShow::allSeasonPosters() const
{
    return m_seasonPosters;
}

const QMap<SeasonNumber, QVector<Poster>>& TvShow::allSeasonBackdrops() const
{
    return m_seasonBackdrops;
}

const QMap<SeasonNumber, QVector<Poster>>& TvShow::allSeasonBanners() const
{
    return m_seasonBanners;
}

const QMap<SeasonNumber, QVector<Poster>>& TvShow::allSeasonThumbs() const
{
    return m_seasonThumbs;
}

TvShowEpisode* TvShow::episode(SeasonNumber season, EpisodeNumber episode)
{
    for (int i = 0, n = m_episodes.count(); i < n; ++i) {
        if (m_episodes[i]->seasonNumber() == season && m_episodes[i]->episodeNumber() == episode) {
            return m_episodes[i];
        }
    }
    return new TvShowEpisode(QStringList(), this);
}

QVector<SeasonNumber> TvShow::seasons(bool includeDummies) const
{
    QVector<SeasonNumber> seasons;
    for (TvShowEpisode* episode : m_episodes) {
        if (episode->isDummy() && !includeDummies) {
            continue;
        }
        if (!seasons.contains(episode->seasonNumber()) && episode->seasonNumber() != SeasonNumber::NoSeason) {
            seasons.append(episode->seasonNumber());
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
        if (episode->seasonNumber() == season) {
            episodes.push_back(episode);
        }
    }

    return episodes;
}

TvShowModelItem* TvShow::modelItem()
{
    return m_modelItem;
}

/**
 * \brief Returns true if something has changed since the last load
 */
bool TvShow::hasChanged() const
{
    return m_hasChanged;
}

mediaelch::DirectoryPath TvShow::mediaCenterPath() const
{
    return m_mediaCenterPath;
}

int TvShow::showId() const
{
    return m_showId;
}

bool TvShow::downloadsInProgress() const
{
    return m_downloadsInProgress;
}

QString TvShow::nfoContent() const
{
    return m_nfoContent;
}

int TvShow::databaseId() const
{
    return m_databaseId;
}

bool TvShow::syncNeeded() const
{
    return m_syncNeeded;
}

QSet<ShowScraperInfo> TvShow::infosToLoad() const
{
    return m_infosToLoad;
}

QSet<EpisodeScraperInfo> TvShow::episodeInfosToLoad() const
{
    return m_episodeInfosToLoad;
}

QStringList TvShow::tags() const
{
    return m_tags;
}

void TvShow::setTitle(const QString& title)
{
    m_title = title.trimmed();
    setChanged(true);
}

void TvShow::setOriginalTitle(const QString& title)
{
    m_originalTitle = title.trimmed();
    setChanged(true);
}

void TvShow::setShowTitle(const QString& title)
{
    m_showTitle = title;
    setChanged(true);
}

void TvShow::setSortTitle(const QString& sortTitle)
{
    m_sortTitle = sortTitle;
    setChanged(true);
}

void TvShow::setUserRating(double rating)
{
    m_userRating = rating;
    setChanged(true);
}

void TvShow::setFirstAired(QDate aired)
{
    m_firstAired = aired;
    setChanged(true);
}

/**
 * \brief Sets all genres
 * \see TvShow::genres
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
 * \brief Adds a genre
 * \see TvShow::genres
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
 * \brief Sets the certification
 * \see TvShow::certification
 */
void TvShow::setCertification(Certification certification)
{
    m_certification = certification;
    setChanged(true);
}

/**
 * \brief Sets the network
 * \see TvShow::network
 */
void TvShow::setNetwork(QString network)
{
    m_network = network;
    setChanged(true);
}

/**
 * \brief Sets the plot
 * \see TvShow::overview
 */
void TvShow::setOverview(QString overview)
{
    m_overview = overview;
    setChanged(true);
}

void TvShow::setTmdbId(TmdbId id)
{
    m_tmdbId = id;
    setChanged(true);
}

void TvShow::setTvdbId(TvDbId id)
{
    m_tvdbId = id;
    setChanged(true);
}

void TvShow::setImdbId(ImdbId id)
{
    m_imdbId = id;
    setChanged(true);
}

void TvShow::setTvMazeId(TvMazeId id)
{
    m_tvmazeId = id;
    setChanged(true);
}

/**
 * \brief Sets the Episode guide url
 * \see TvShow::episodeGuideUrl
 */
void TvShow::setEpisodeGuideUrl(QString url)
{
    m_episodeGuideUrl = url;
    setChanged(true);
}

/// \brief Adds an actor
/// \see TvShow::actors
void TvShow::addActor(Actor actor)
{
    if (actor.order == 0 && !m_actors.empty()) {
        actor.order = m_actors.back()->order + 1;
    }
    m_actors.push_back(std::make_unique<Actor>(actor));
    setChanged(true);
}

/**
 * \brief Sets all posters
 * \see TvShow::posters
 */
void TvShow::setPosters(QVector<Poster> posters)
{
    m_posters = posters;
    setChanged(true);
}

/**
 * \brief Sets all banners
 * \see TvShow::banners
 */
void TvShow::setBanners(QVector<Poster> banners)
{
    m_banners = banners;
    setChanged(true);
}

/**
 * \brief Sets the poster for a specific index
 * \see TvShow::posters
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
 * \brief Appends a list of backdrops
 * \see TvShow::backdrops
 */
void TvShow::setBackdrops(QVector<Poster> backdrops)
{
    m_backdrops.append(backdrops);
    setChanged(true);
}

/**
 * \brief Sets the backdrop for a specific index
 * \see TvShow::backdrops
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
 * \brief Adds a poster
 * \see TvShow::posters
 */
void TvShow::addPoster(Poster poster)
{
    m_posters.append(poster);
    setChanged(true);
}

/**
 * \brief Adds a banner
 * \see TvShow::banners
 */
void TvShow::addBanner(Poster banner)
{
    m_banners.append(banner);
    setChanged(true);
}

/**
 * \brief Adds a backdrop
 * \see TvShow::backdrops
 */
void TvShow::addBackdrop(Poster backdrop)
{
    m_backdrops.append(backdrop);
    setChanged(true);
}

/**
 * \brief Adds a new season poster
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

void TvShow::setChanged(bool changed)
{
    m_hasChanged = changed;
    emit sigChanged(this);
}

void TvShow::setModelItem(TvShowModelItem* item)
{
    if (item == nullptr) {
        qCritical() << "[TvShow] Tried to set nullptr model item";
        return;
    }
    m_modelItem = item;
}

void TvShow::setMediaCenterPath(mediaelch::DirectoryPath path)
{
    m_mediaCenterPath = path;
}

void TvShow::setDownloadsInProgress(bool inProgress)
{
    m_downloadsInProgress = inProgress;
}

/*** REMOVER ***/

/**
 * \brief Removes an actor
 * \param actor Pointer to the actor to remove
 * \see TvShow::actors
 */
void TvShow::removeActor(Actor* actor)
{
    for (int i = 0, n = m_actors.size(); i < n; ++i) {
        if (m_actors[i].get() == actor) {
            m_actors.erase(m_actors.begin() + i);
            break;
        }
    }
    setChanged(true);
}

/**
 * \brief Removes a genre
 * \param genre Genre to remove
 * \see TvShow::genres
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

void TvShow::setNfoContent(QString content)
{
    m_nfoContent = content;
}

void TvShow::setDatabaseId(int id)
{
    m_databaseId = id;
}

void TvShow::setSyncNeeded(bool syncNeeded)
{
    m_syncNeeded = syncNeeded;
    emit sigChanged(this);
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
    if (m_extraFanarts.isEmpty() && mediaCenterInterface != nullptr) {
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
    return (QString::localeAwareCompare(helper::appendArticle(a->title()), helper::appendArticle(b->title())) < 0);
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

void TvShow::setDir(const mediaelch::DirectoryPath& dir)
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

QDateTime TvShow::dateAdded() const
{
    return m_dateAdded;
}

void TvShow::setDateAdded(const QDateTime& dateTime)
{
    m_dateAdded = dateTime;
    setChanged(true);
}

const QMap<SeasonNumber, QString>& TvShow::seasonNameMappings() const
{
    return m_seasonNameMappings;
}

void TvShow::setSeasonName(SeasonNumber season, const QString& name)
{
    m_seasonNameMappings.insert(season, name);
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

bool TvShow::isDummySeason(SeasonNumber season) const
{
    for (TvShowEpisode* episode : m_episodes) {
        if (episode->seasonNumber() == season && !episode->isDummy()) {
            return false;
        }
    }
    return true;
}

bool TvShow::hasDummyEpisodes(SeasonNumber season) const
{
    return std::any_of(m_episodes.cbegin(), m_episodes.cend(), [season](const TvShowEpisode* const episode) {
        return episode->seasonNumber() == season && episode->isDummy();
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
            if (m_episodes[i]->seasonNumber() == episode->seasonNumber()
                && m_episodes[i]->episodeNumber() == episode->episodeNumber()) {
                found = true;
                break;
            }
        }

        if (found) {
            episode->deleteLater();
            continue;
        }

        if (episode->seasonNumber() == SeasonNumber::SpecialsSeason && hideSpecialsInMissingEpisodes()) {
            episode->deleteLater();
            continue;
        }

        episode->loadData(Manager::instance()->mediaCenterInterfaceTvShow(), false, false);
        episode->setIsDummy(true);
        episode->setInfosLoaded(true);
        addEpisode(episode);
    }

    Manager::instance()->tvShowModel()->updateShow(this);
    TvShowFilesWidget::instance().renewModel(true);
}

void TvShow::clearMissingEpisodes()
{
    const auto isDummyEpisode = [](TvShowEpisode* episode) { return episode->isDummy(); };
    m_episodes.erase(std::remove_if(m_episodes.begin(), m_episodes.end(), isDummyEpisode), m_episodes.end());

    Manager::instance()->tvShowModel()->updateShow(this);
    TvShowFilesWidget::instance().renewModel(true);
}

QDebug operator<<(QDebug dbg, const TvShow& show)
{
    QDebugStateSaver saver(dbg);

    QString nl = "\n";
    QString out;
    out.append("TvShow").append(nl);
    out.append(QStringLiteral("  Dir:           ").append(show.dir().toString()).append(nl));
    out.append(QStringLiteral("  Name:          ").append(show.title()).append(nl));
    out.append(QStringLiteral("  OriginalTitle: ").append(show.originalTitle()).append(nl));
    out.append(QStringLiteral("  ShowTitle:     ").append(show.showTitle()).append(nl));
    out.append(QStringLiteral("  SortTitle:     ").append(show.sortTitle()).append(nl));
    out.append(QString("  Ratings:").append(nl));
    for (const Rating& rating : show.ratings()) {
        out.append(
            QString("    %1: %2 (%3 votes)").arg(rating.source).arg(rating.rating).arg(rating.voteCount).append(nl));
    }
    out.append(QStringLiteral("  FirstAired:    ").append(show.firstAired().toString("yyyy-MM-dd")).append(nl));
    out.append(QStringLiteral("  Certification: ").append(show.certification().toString()).append(nl));
    out.append(QStringLiteral("  Network:       ").append(show.network()).append(nl));
    out.append(QStringLiteral("  Overview:      ").append(show.overview())).append(nl);
    out.append(QStringLiteral("  Status:        ").append(show.status())).append(nl);
    const auto& genres = show.genres();
    for (const QString& genre : genres) {
        out.append(QString("  Genre:         ").append(genre)).append(nl);
    }
    for (const Actor* actor : show.actors()) {
        out.append(QStringLiteral("  Actor:         ").append(nl));
        out.append(QStringLiteral("    Name:  ").append(actor->name)).append(nl);
        out.append(QStringLiteral("    Role:  ").append(actor->role)).append(nl);
        out.append(QStringLiteral("    Thumb: ").append(actor->thumb)).append(nl);
    }
    out.append(QStringLiteral("  User-Rating:   ").append(QString::number(show.userRating())).append(nl));

    for (const Poster& poster : show.posters()) {
        out.append(QStringLiteral("  Poster:       ")).append(nl);
        out.append(QStringLiteral("    ID:       ").append(poster.id)).append(nl);
        out.append(QStringLiteral("    Original: ").append(poster.originalUrl.toString())).append(nl);
        out.append(QStringLiteral("    Thumb:    ").append(poster.thumbUrl.toString())).append(nl);
    }

    for (const Poster& backdrop : show.backdrops()) {
        out.append(QStringLiteral("  Backdrop:       ")).append(nl);
        out.append(QStringLiteral("    ID:       ").append(backdrop.id)).append(nl);
        out.append(QStringLiteral("    Original: ").append(backdrop.originalUrl.toString())).append(nl);
        out.append(QStringLiteral("    Thumb:    ").append(backdrop.thumbUrl.toString())).append(nl);
    }

    dbg.nospace().noquote() << out;
    return dbg;
}

QDebug operator<<(QDebug dbg, const TvShow* show)
{
    dbg << *show;
    return dbg;
}
