#include "TvShowEpisode.h"

#include "data/tv_show/TvShow.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "log/Log.h"
#include "media_center/MediaCenterInterface.h"
#include "model/tv_show/EpisodeModelItem.h"
#include "model/tv_show/TvShowUtils.h"
#include "scrapers/tv_show/ShowMerger.h"
#include "scrapers/tv_show/TvScraper.h"
#include "settings/Settings.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTime>
#include <utility>

TvShowEpisode::TvShowEpisode(const mediaelch::FileList& files, QObject* parent) :
    QObject(parent),
    m_season{SeasonNumber::NoSeason},
    m_episode{EpisodeNumber::NoEpisode},
    m_displaySeason{SeasonNumber::NoSeason},
    m_displayEpisode{EpisodeNumber::NoEpisode}
{
    initCounter();
    setFiles(files);
}

TvShowEpisode::TvShowEpisode(const mediaelch::FileList& files, TvShow* parentShow) :
    QObject(parentShow),
    m_show{parentShow},
    m_season{SeasonNumber::NoSeason},
    m_episode{EpisodeNumber::NoEpisode},
    m_displaySeason{SeasonNumber::NoSeason},
    m_displayEpisode{EpisodeNumber::NoEpisode}
{
    initCounter();
    setFiles(files);
}

void TvShowEpisode::initCounter()
{
    static int m_idCounter = 0;
    m_episodeId = ++m_idCounter;
}

void TvShowEpisode::setFiles(const mediaelch::FileList& files)
{
    m_files = files;
    m_streamDetails = new StreamDetails(this, m_files);
}

void TvShowEpisode::setShow(TvShow* show)
{
    m_show = show;
    setParent(show);
}

void TvShowEpisode::clear()
{
    m_imdbId = {};
    m_tvdbId = {};
    m_tvmazeId = {};

    QSet<EpisodeScraperInfo> infos;
    infos << EpisodeScraperInfo::Title         //
          << EpisodeScraperInfo::Certification //
          << EpisodeScraperInfo::Rating        //
          << EpisodeScraperInfo::Director      //
          << EpisodeScraperInfo::Writer        //
          << EpisodeScraperInfo::Overview      //
          << EpisodeScraperInfo::Network       //
          << EpisodeScraperInfo::Title         //
          << EpisodeScraperInfo::FirstAired    //
          << EpisodeScraperInfo::Tags          //
          << EpisodeScraperInfo::Thumbnail     //
          << EpisodeScraperInfo::Actors;
    clear(infos);
    m_nfoContent.clear();
}

void TvShowEpisode::clear(const QSet<EpisodeScraperInfo>& infos)
{
    if (infos.contains(EpisodeScraperInfo::Certification)) {
        m_certification = Certification::NoCertification;
    }
    if (infos.contains(EpisodeScraperInfo::Rating)) {
        m_ratings.clear();
    }
    if (infos.contains(EpisodeScraperInfo::Director)) {
        m_directors.clear();
    }
    if (infos.contains(EpisodeScraperInfo::Writer)) {
        m_writers.clear();
    }
    if (infos.contains(EpisodeScraperInfo::Overview)) {
        m_overview = "";
    }
    if (infos.contains(EpisodeScraperInfo::Network)) {
        m_networks.clear();
    }
    if (infos.contains(EpisodeScraperInfo::Title)) {
        m_title.clear();
        m_showTitle.clear();
    }
    if (infos.contains(EpisodeScraperInfo::Tags)) {
        m_tags.clear();
    }
    if (infos.contains(EpisodeScraperInfo::FirstAired)) {
        m_firstAired = QDate(2000, 02, 30); // invalid date;
    }
    if (infos.contains(EpisodeScraperInfo::Thumbnail)) {
        m_thumbnail = QUrl();
        m_thumbnailImageChanged = false;
        m_imagesToRemove.remove(ImageType::TvShowEpisodeThumb);
    }
    if (infos.contains(EpisodeScraperInfo::Actors)) {
        m_actors.removeAll();
    }

    m_hasChanged = false;
}

void TvShowEpisode::exportTo(Exporter& exporter) const
{
    exporter.startExport();

    exporter.exportEpisodeId(m_episodeId);
    exporter.exportDatabaseId(m_databaseId);
    exporter.exportTmdbId(m_tmdbId);
    exporter.exportImdbId(m_imdbId);
    exporter.exportTvdbId(m_tvdbId);
    exporter.exportTvMazeId(m_tvmazeId);

    exporter.exportTitle(m_title);
    exporter.exportShowTitle(m_showTitle);

    exporter.exportRatings(m_ratings);
    exporter.exportUserRating(m_userRating);
    exporter.exportImdbTop250(m_imdbTop250);

    exporter.exportSeason(m_season);
    exporter.exportEpisode(m_episode);
    exporter.exportDisplaySeason(m_displaySeason);
    exporter.exportDisplayEpisode(m_displayEpisode);

    exporter.exportOverview(m_overview);
    exporter.exportWriters(m_writers);
    exporter.exportDirectors(m_directors);
    exporter.exportPlayCount(m_playCount);
    exporter.exportLastPlayed(m_lastPlayed);
    exporter.exportFirstAired(m_firstAired);
    exporter.exportTags(m_tags);
    exporter.exportEpBookmark(m_epBookmark);
    exporter.exportCertification(m_certification);
    exporter.exportNetworks(m_networks);
    exporter.exportThumbnail(m_thumbnail);
    exporter.exportActors(m_actors);
    exporter.exportStreamDetails(m_streamDetails);
    exporter.exportFiles(m_files);

    exporter.endExport();
}

QSet<EpisodeScraperInfo> TvShowEpisode::infosToLoad()
{
    return m_infosToLoad;
}

/**
 * \brief Load data using a MediaCenterInterface
 * \param mediaCenterInterface MediaCenterInterface to use
 * \return Loading was successful
 */
bool TvShowEpisode::loadData(MediaCenterInterface* mediaCenterInterface, bool reloadFromNfo, bool forceReload)
{
    if (mediaCenterInterface == nullptr) {
        qCWarning(generic) << "Passed an empty (null) mediaCenterInterface to loadData";
        return false;
    }

    if (!forceReload && (m_infoLoaded || !hasChanged()) && m_infoFromNfoLoaded) {
        return m_infoLoaded;
    }

    const bool infoLoaded = [&]() {
        return reloadFromNfo ? mediaCenterInterface->loadTvShowEpisode(this)
                             : mediaCenterInterface->loadTvShowEpisode(this, nfoContent());
    }();

    // TV episodes should always have a name, even if a valid NFO file does not have a title tag.
    if ((!infoLoaded || m_title.isEmpty()) && !files().isEmpty()) {
        setTitle(mediaelch::guessTvShowTitleFromFiles(files()));
    }
    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    setChanged(false);
    return infoLoaded;
}

bool TvShowEpisode::loadStreamDetailsFromFile()
{
    const bool success = m_streamDetails->loadStreamDetails();
    if (success) {
        setChanged(true);
    }
    return success;
}

/**
 * \brief Save data using a MediaCenterInterface
 * \param mediaCenterInterface MediaCenterInterface to use
 * \return Saving was successful
 */
bool TvShowEpisode::saveData(MediaCenterInterface* mediaCenterInterface)
{
    if (!streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails()) {
        const bool success = loadStreamDetailsFromFile();
        if (!success) {
            // TODO: Tell the user that it failed
            qCDebug(generic) << "[TvShowEpisode] Could not load stream details for episode with ID=" << m_episodeId;
        }
    }
    bool saved = mediaCenterInterface->saveTvShowEpisode(this);
    qCDebug(generic) << "[TvShowEpisode] Saving episode" << (saved ? "successful:" : "NOT successful:") << "Season"
                     << m_season.toPaddedString() << "Episode" << m_episode.toPaddedString();
    if (!m_infoLoaded) {
        m_infoLoaded = saved;
    }
    setSyncNeeded(true);
    setChanged(false);
    clearImages();
    return saved;
}

void TvShowEpisode::scrapeData(mediaelch::scraper::TvScraper* scraper,
    mediaelch::Locale locale,
    const mediaelch::scraper::ShowIdentifier& showIdentifier,
    SeasonOrder order,
    const QSet<EpisodeScraperInfo>& infosToLoad)
{
    using namespace mediaelch;
    using namespace mediaelch::scraper;

    qCInfo(generic) << "[TvShow] Load episode with show id" << showIdentifier << "using scraper"
                    << scraper->meta().name;
    m_infosToLoad = infosToLoad;

    EpisodeIdentifier identifier(showIdentifier.str(), seasonNumber(), episodeNumber(), order);
    EpisodeScrapeJob::Config config(identifier, locale, infosToLoad);
    auto* scrapeJob = scraper->loadEpisode(config);
    connect(scrapeJob, &scraper::EpisodeScrapeJob::loadFinished, this, [this](scraper::EpisodeScrapeJob* job) {
        // Map according to advanced settings
        const QStringList networks = helper::mapStudio(job->episode().networks());
        const Certification certification = helper::mapCertification(job->episode().certification());

        job->episode().setNetworks(networks);
        job->episode().setCertification(certification);

        clear(job->config().details);
        scraper::copyDetailsToEpisode(*this, job->episode(), job->config().details);
        job->deleteLater();
        emit sigLoaded(this);
    });
    scrapeJob->start();
}

TvShow* TvShowEpisode::tvShow() const
{
    return m_show;
}

bool TvShowEpisode::isValid() const
{
    return !m_files.isEmpty();
}

/**
 * \brief Clears the episode images to save memory
 */
void TvShowEpisode::clearImages()
{
    m_thumbnailImage = QByteArray();
}

bool TvShowEpisode::infoLoaded() const
{
    return m_infoLoaded;
}

QString TvShowEpisode::title() const
{
    return m_title;
}

QString TvShowEpisode::completeEpisodeName() const
{
    return QString("S%1E%2 %3").arg(seasonString()).arg(episodeString()).arg(title());
}

const mediaelch::FileList& TvShowEpisode::files() const
{
    return m_files;
}

/**
 * \property TvShow::showTitle
 * \brief The title of the parent show
 * \return Title
 */
QString TvShowEpisode::showTitle() const
{
    if (!m_showTitle.isEmpty()) {
        return m_showTitle;
    }
    if (m_show != nullptr) {
        return m_show->title();
    }

    return QString();
}

Ratings& TvShowEpisode::ratings()
{
    return m_ratings;
}

const Ratings& TvShowEpisode::ratings() const
{
    return m_ratings;
}

double TvShowEpisode::userRating() const
{
    return m_userRating;
}

/**
 * \property TvShowEpisode::season
 * \brief Season number
 * \return Season number
 * \see TvShowEpisode::setSeasonNumber
 */
SeasonNumber TvShowEpisode::seasonNumber() const
{
    return m_season;
}

/**
 * \property TvShowEpisode::displaySeason
 * \brief Display Season number
 * \return Display Season number
 * \see TvShowEpisode::setDisplaySeasonNumber
 */
SeasonNumber TvShowEpisode::displaySeason() const
{
    return m_displaySeason;
}

/**
 * \brief Season number with leading zero
 * \return Formatted Season number
 * \see TvShowEpisode::season
 */
QString TvShowEpisode::seasonString() const
{
    return seasonNumber().toPaddedString();
}

/**
 * \brief Season name
 * \return Season name or empty
 */
QString TvShowEpisode::seasonName() const
{
    if (m_show != nullptr) {
        return m_show->seasonNameMappings().value(seasonNumber(), "");
    } else {
        return {};
    }
}

/**
 * \property TvShowEpisode::episode
 * \brief Episode number
 * \return Episode number
 * \see TvShowEpisode::setEpisode
 */
EpisodeNumber TvShowEpisode::episodeNumber() const
{
    return m_episode;
}

/**
 * \property TvShowEpisode::displayEpisode
 * \brief Display Episode number
 * \return Display Episode number
 * \see TvShowEpisode::setDisplayEpisode
 */
EpisodeNumber TvShowEpisode::displayEpisode() const
{
    return m_displayEpisode;
}

/**
 * \brief Episode number with leading zero
 * \return Formatted episode number
 * \see TvShowEpisode::setEpisode
 */
QString TvShowEpisode::episodeString() const
{
    return episodeNumber().toPaddedString();
}

/**
 * \property TvShowEpisode::overview
 * \brief Plot of the episode
 * \return Plot
 * \see TvShowEpisode::setOverview
 */
QString TvShowEpisode::overview() const
{
    return m_overview;
}

/**
 * \property TvShowEpisode::writers
 * \brief Writers of the episode
 * \return List of writers
 * \see TvShowEpisode::setWriters
 */
QStringList TvShowEpisode::writers() const
{
    return m_writers;
}

/**
 * \property TvShowEpisode::directors
 * \brief Directors of the episode
 * \return List of directors
 * \see TvShowEpisode::setDirectors
 */
QStringList TvShowEpisode::directors() const
{
    return m_directors;
}

/**
 * \property TvShowEpisode::playCount
 * \brief Playcount of the episode
 * \return Playcount
 * \see TvShowEpisode::setPlayCount
 */
int TvShowEpisode::playCount() const
{
    return m_playCount;
}

/**
 * \property TvShowEpisode::lastPlayed
 * \brief Holds the last time the episode was played
 * \return Last playtime
 * \see TvShowEpisode::setLastPlayed
 */
QDateTime TvShowEpisode::lastPlayed() const
{
    return m_lastPlayed;
}

/**
 * \property TvShowEpisode::firstAired
 * \brief Holds the first aired date of the episode
 * \return First aired
 * \see TvShowEpisode::setFirstAired
 */
QDate TvShowEpisode::firstAired() const
{
    return m_firstAired;
}

/**
 * \property TvShowEpisode::certification
 * \brief Certification of the episode
 * \return Certification
 * \see TvShowEpisode::setCertification
 */
Certification TvShowEpisode::certification() const
{
    if (m_certification.isValid()) {
        return m_certification;
    }
    if (m_show != nullptr) {
        return m_show->certification();
    }

    return Certification::NoCertification;
}

QStringList TvShowEpisode::networks() const
{
    if (!m_networks.isEmpty()) {
        return m_networks;
    }
    if (m_show != nullptr) {
        return m_show->networks();
    }

    return {};
}

/**
 * \brief Holds the thumbnail url
 * \return URL of the thumbnail
 */
QUrl TvShowEpisode::thumbnail() const
{
    return m_thumbnail;
}

/**
 * \brief Holds the current thumbnail image
 * \return Image of the thumbnail
 */
QByteArray TvShowEpisode::thumbnailImage()
{
    return m_thumbnailImage;
}

bool TvShowEpisode::thumbnailImageChanged() const
{
    return m_thumbnailImageChanged;
}

EpisodeModelItem* TvShowEpisode::modelItem()
{
    return m_modelItem;
}

bool TvShowEpisode::hasChanged() const
{
    return m_hasChanged;
}

/**
 * \brief Returns a list of pointer to the writers
 * \return List of pointers
 */
QVector<QString*> TvShowEpisode::writersPointer()
{
    QVector<QString*> writers;
    for (elch_ssize_t i = 0, n = m_writers.size(); i < n; ++i) {
        writers.append(&m_writers[i]);
    }
    return writers;
}

/**
 * \brief Returns a list of pointers to the directors
 * \return List of pointers
 */
QVector<QString*> TvShowEpisode::directorsPointer()
{
    QVector<QString*> directors;
    for (elch_ssize_t i = 0, n = m_directors.size(); i < n; ++i) {
        directors.append(&m_directors[i]);
    }
    return directors;
}

int TvShowEpisode::episodeId() const
{
    return m_episodeId;
}

/**
 * \property TvShowEpisode::streamDetailsLoaded
 * \brief Holds if the stream details were loaded
 * \return True if the stream details were loaded
 */
bool TvShowEpisode::streamDetailsLoaded() const
{
    return m_streamDetails->hasLoaded();
}

/**
 * \brief The stream details object of this episode
 * \return StreamDetails Object
 */
StreamDetails* TvShowEpisode::streamDetails()
{
    return m_streamDetails;
}

const StreamDetails* TvShowEpisode::streamDetails() const
{
    return m_streamDetails;
}

QString TvShowEpisode::nfoContent() const
{
    return m_nfoContent;
}

mediaelch::DatabaseId TvShowEpisode::databaseId() const
{
    return m_databaseId;
}

bool TvShowEpisode::syncNeeded() const
{
    return m_syncNeeded;
}

QTime TvShowEpisode::epBookmark() const
{
    return m_epBookmark;
}

QStringList TvShowEpisode::tags() const
{
    return m_tags;
}

/*** SETTER ***/

/**
 * \brief Sets the name
 * \param title Name of the episode
 * \see TvShowEpisode::name
 */
void TvShowEpisode::setTitle(QString title)
{
    m_title = title;
    setChanged(true);
}

/**
 * \brief Sets the title of the show
 */
void TvShowEpisode::setShowTitle(QString showTitle)
{
    m_showTitle = showTitle;
    setChanged(true);
}

void TvShowEpisode::setUserRating(double rating)
{
    m_userRating = rating;
    setChanged(true);
}

/**
 * \brief Sets the season
 * \param season Season number
 * \see TvShowEpisode::season
 */
void TvShowEpisode::setSeason(SeasonNumber season)
{
    m_season = season;
    setChanged(true);
}

/**
 * \brief Sets the episode
 * \param episode Episode number
 * \see TvShowEpisode::episode
 */
void TvShowEpisode::setEpisode(EpisodeNumber episode)
{
    m_episode = episode;
    setChanged(true);
}

/**
 * \brief Sets the display season
 * \param season Display Season number
 * \see TvShowEpisode::displaySeason
 */
void TvShowEpisode::setDisplaySeason(SeasonNumber season)
{
    m_displaySeason = season;
    setChanged(true);
}

/**
 * \brief Sets the display episode
 * \param episode Display Episode number
 * \see TvShowEpisode::displayEpisode
 */
void TvShowEpisode::setDisplayEpisode(EpisodeNumber episode)
{
    m_displayEpisode = episode;
    setChanged(true);
}

/**
 * \brief Sets the plot
 * \param overview Plot
 * \see TvShowEpisode::overview
 */
void TvShowEpisode::setOverview(QString overview)
{
    m_overview = overview;
    setChanged(true);
}

void TvShowEpisode::setEpBookmark(QTime epBookmark)
{
    m_epBookmark = epBookmark;
    setChanged(true);
}

/**
 * \brief Sets all writers
 * \param writers List of writers
 * \see TvShowEpisode::writers
 */
void TvShowEpisode::setWriters(QStringList writers)
{
    m_writers = writers;
    setChanged(true);
}

/**
 * \brief Adds a writer
 * \param writer Writer to add
 * \see TvShowEpisode::writers
 */
void TvShowEpisode::addWriter(QString writer)
{
    m_writers.append(writer);
    setChanged(true);
}

/**
 * \brief Adds a director
 * \param director Director to add
 * \see TvShowEpisode::directors
 */
void TvShowEpisode::addDirector(QString director)
{
    m_directors.append(director);
    setChanged(true);
}

void TvShowEpisode::addTag(QString tag)
{
    m_tags.append(tag);
    setChanged(true);
}

/**
 * \brief Sets all directors
 * \param directors List of directors
 * \see TvShowEpisode::directors
 */
void TvShowEpisode::setDirectors(QStringList directors)
{
    m_directors = directors;
    setChanged(true);
}

void TvShowEpisode::setPlayCount(int playCount)
{
    m_playCount = playCount;
    setChanged(true);
}

/**
 * \brief Sets the last playtime
 * \param lastPlayed Last playtime
 * \see TvShowEpisode::lastPlayed
 */
void TvShowEpisode::setLastPlayed(QDateTime lastPlayed)
{
    m_lastPlayed = lastPlayed;
    setChanged(true);
}

void TvShowEpisode::setFirstAired(QDate firstAired)
{
    m_firstAired = firstAired;
    setChanged(true);
}

void TvShowEpisode::setCertification(Certification certification)
{
    m_certification = certification;
    setChanged(true);
}

void TvShowEpisode::setNetworks(QStringList networks)
{
    m_networks = networks;
    setChanged(true);
}

void TvShowEpisode::addNetwork(QString network)
{
    if (network.isEmpty()) {
        return;
    }
    m_networks.append(network);
    setChanged(true);
}

void TvShowEpisode::setThumbnail(QUrl url)
{
    m_thumbnail = url;
    setChanged(true);
}

void TvShowEpisode::setThumbnailImage(QByteArray thumbnail)
{
    m_thumbnailImage = thumbnail;
    m_thumbnailImageChanged = true;
    setChanged(true);
}

void TvShowEpisode::setInfosLoaded(bool loaded)
{
    m_infoLoaded = loaded;
}

void TvShowEpisode::setChanged(bool changed)
{
    m_hasChanged = changed;
    emit sigChanged(this);
}

void TvShowEpisode::setModelItem(EpisodeModelItem* item)
{
    m_modelItem = item;
}

void TvShowEpisode::removeWriter(QString* writer)
{
    for (elch_ssize_t i = 0, n = m_writers.size(); i < n; ++i) {
        if (&m_writers[i] == writer) {
            m_writers.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

/**
 * \brief Removes a director
 * \see TvShowEpisode::directors
 */
void TvShowEpisode::removeDirector(QString* director)
{
    for (elch_ssize_t i = 0, n = m_directors.size(); i < n; ++i) {
        if (&m_directors[i] == director) {
            m_directors.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

void TvShowEpisode::removeTag(QString tag)
{
    m_tags.removeAll(tag);
    setChanged(true);
}

void TvShowEpisode::setNfoContent(QString content)
{
    m_nfoContent = content;
}

void TvShowEpisode::setDatabaseId(mediaelch::DatabaseId id)
{
    m_databaseId = id;
}

void TvShowEpisode::setSyncNeeded(bool syncNeeded)
{
    m_syncNeeded = syncNeeded;
}

QSet<ImageType> TvShowEpisode::imagesToRemove() const
{
    return m_imagesToRemove;
}

void TvShowEpisode::removeImage(ImageType type)
{
    if (type == ImageType::TvShowEpisodeThumb) {
        if (!m_thumbnailImage.isNull()) {
            m_thumbnailImage = QByteArray();
            m_thumbnailImageChanged = false;
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.insert(type);
        }
    }
    setChanged(true);
}

void TvShowEpisode::setIsDummy(bool dummy)
{
    m_isDummy = dummy;
}

bool TvShowEpisode::isDummy() const
{
    return m_isDummy;
}

const Actors& TvShowEpisode::actors() const
{
    return m_actors;
}

Actors& TvShowEpisode::actors()
{
    return m_actors;
}

void TvShowEpisode::addActor(Actor actor)
{
    m_actors.addActor(actor);
    setChanged(true);
}

void TvShowEpisode::removeActor(Actor* actor)
{
    m_actors.removeActor(actor);
    setChanged(true);
}

bool TvShowEpisode::lessThan(TvShowEpisode* a, TvShowEpisode* b)
{
    if (a->seasonNumber() < b->seasonNumber()) {
        return true;
    }
    if (a->seasonNumber() > b->seasonNumber()) {
        return false;
    }
    if (a->episodeNumber() < b->episodeNumber()) {
        return true;
    }
    if (a->episodeNumber() > b->episodeNumber()) {
        return false;
    }

    return (QString::localeAwareCompare(helper::appendArticle(a->title()), helper::appendArticle(b->title())) < 0);
}

TmdbId TvShowEpisode::tmdbId() const
{
    return m_tmdbId;
}

ImdbId TvShowEpisode::imdbId() const
{
    return m_imdbId;
}

TvDbId TvShowEpisode::tvdbId() const
{
    return m_tvdbId;
}

void TvShowEpisode::setTmdbId(const TmdbId& tmdbId)
{
    m_tmdbId = tmdbId;
    setChanged(true);
}

void TvShowEpisode::setImdbId(const ImdbId& imdbId)
{
    m_imdbId = imdbId;
    setChanged(true);
}

void TvShowEpisode::setTvdbId(const TvDbId& tvdbId)
{
    m_tvdbId = tvdbId;
    setChanged(true);
}

TvMazeId TvShowEpisode::tvmazeId() const
{
    return m_tvmazeId;
}

void TvShowEpisode::setTvMazeId(const TvMazeId& tvmazeId)
{
    m_tvmazeId = tvmazeId;
    setChanged(true);
}

int TvShowEpisode::top250() const
{
    return m_imdbTop250;
}

void TvShowEpisode::setTop250(int top250)
{
    m_imdbTop250 = top250;
    setChanged(true);
}

/*** DEBUG ***/

QDebug operator<<(QDebug dbg, const TvShowEpisode& episode)
{
    QString nl = "\n";
    QString out;
    out.append("TvShowEpisode").append(nl);
    out.append(QString("  Files:         ").append(nl));
    for (const mediaelch::FilePath& file : episode.files()) {
        out.append(QString("    %1").arg(file.toNativePathString()).append(nl));
    }
    out.append(QStringLiteral("  Name:          ").append(episode.title()).append(nl));
    out.append(QStringLiteral("  ShowTitle:     ").append(episode.showTitle()).append(nl));
    out.append(QStringLiteral("  Season:        %1").arg(episode.seasonNumber().toPaddedString()).append(nl));
    out.append(QStringLiteral("  Episode:       %1").arg(episode.episodeNumber().toPaddedString()).append(nl));
    out.append(QString("  Ratings:").append(nl));
    for (const Rating& rating : episode.ratings()) {
        out.append(
            QString("    %1: %2 (%3 votes)").arg(rating.source).arg(rating.rating).arg(rating.voteCount).append(nl));
    }
    out.append(QString("  User Rating:   ").append(QString::number(episode.userRating())).append(nl));
    out.append(QStringLiteral("  FirstAired:    ").append(episode.firstAired().toString("yyyy-MM-dd")).append(nl));
    out.append(QStringLiteral("  LastPlayed:    ").append(episode.lastPlayed().toString("yyyy-MM-dd")).append(nl));
    out.append(QStringLiteral("  Playcount:     %1%2").arg(episode.playCount()).arg(nl));
    out.append(QStringLiteral("  Certification: ").append(episode.certification().toString()).append(nl));
    out.append(QStringLiteral("  Overview:      ").append(episode.overview())).append(nl);
    for (const QString& writer : episode.writers()) {
        out.append(QString("  Writer:        ").append(writer)).append(nl);
    }
    for (const QString& director : episode.directors()) {
        out.append(QString("  Director:      ").append(director)).append(nl);
    }

    for (const Actor* actor : episode.actors()) {
        out.append(QString("  Actor:         ").append(nl));
        out.append(QString("    Name:  ").append(actor->name)).append(nl);
        out.append(QString("    Role:  ").append(actor->role)).append(nl);
        out.append(QString("    Thumb: ").append(actor->thumb)).append(nl);
    }
    dbg.nospace() << out;
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const TvShowEpisode* episode)
{
    dbg.nospace() << *episode;
    return dbg.space();
}
