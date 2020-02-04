#include "TvShowEpisode.h"

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "media_centers/MediaCenterInterface.h"
#include "scrapers/tv_show/TvScraperInterface.h"
#include "settings/Settings.h"
#include "tv_shows/TvShow.h"
#include "tv_shows/model/EpisodeModelItem.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTime>
#include <utility>

/**
 * @brief TvShowEpisode::TvShowEpisode
 * @param files Files of the episode
 */
TvShowEpisode::TvShowEpisode(QStringList files, TvShow* parent) :
    QObject(parent),
    m_parent{parent},
    m_season{SeasonNumber::NoSeason},
    m_episode{EpisodeNumber::NoEpisode},
    m_displaySeason{SeasonNumber::NoSeason},
    m_displayEpisode{EpisodeNumber::NoEpisode},
    m_playCount{0},
    m_thumbnailImageChanged{false},
    m_infoLoaded{false},
    m_infoFromNfoLoaded{false},
    m_hasChanged{false},
    m_streamDetailsLoaded{false},
    m_databaseId{-1},
    m_syncNeeded{false},
    m_isDummy{false}
{
    static int m_idCounter = 0;
    m_episodeId = ++m_idCounter;
    setFiles(files);
}

void TvShowEpisode::setFiles(QStringList files)
{
    m_files = files;
    if (!files.isEmpty()) {
        m_streamDetails = new StreamDetails(this, files);
    } else {
        m_streamDetails = new StreamDetails(this, QStringList());
    }
}

void TvShowEpisode::setShow(TvShow* show)
{
    m_parent = show;
    setParent(show);
}

/**
 * @brief Clears the episodes data
 */
void TvShowEpisode::clear()
{
    QVector<TvShowScraperInfos> infos;
    infos << TvShowScraperInfos::Certification //
          << TvShowScraperInfos::Rating        //
          << TvShowScraperInfos::Director      //
          << TvShowScraperInfos::Writer        //
          << TvShowScraperInfos::Overview      //
          << TvShowScraperInfos::Network       //
          << TvShowScraperInfos::FirstAired    //
          << TvShowScraperInfos::Thumbnail     //
          << TvShowScraperInfos::Actors;
    clear(infos);
    m_nfoContent.clear();
}

void TvShowEpisode::clear(QVector<TvShowScraperInfos> infos)
{
    if (infos.contains(TvShowScraperInfos::Certification)) {
        m_certification = Certification::NoCertification;
    }
    if (infos.contains(TvShowScraperInfos::Rating)) {
        m_ratings.clear();
    }
    if (infos.contains(TvShowScraperInfos::Director)) {
        m_directors.clear();
    }
    if (infos.contains(TvShowScraperInfos::Writer)) {
        m_writers.clear();
    }
    if (infos.contains(TvShowScraperInfos::Overview)) {
        m_overview = "";
    }
    if (infos.contains(TvShowScraperInfos::Network)) {
        m_network = "";
    }
    if (infos.contains(TvShowScraperInfos::FirstAired)) {
        m_firstAired = QDate(2000, 02, 30); // invalid date;
    }
    if (infos.contains(TvShowScraperInfos::Thumbnail)) {
        m_thumbnail = QUrl();
        m_thumbnailImageChanged = false;
        m_imagesToRemove.removeOne(ImageType::TvShowEpisodeThumb);
    }
    if (infos.contains(TvShowScraperInfos::Actors)) {
        m_actors.clear();
    }

    m_hasChanged = false;
}

QVector<TvShowScraperInfos> TvShowEpisode::infosToLoad()
{
    return m_infosToLoad;
}

/**
 * @brief Load data using a MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use
 * @return Loading was successful
 */
bool TvShowEpisode::loadData(MediaCenterInterface* mediaCenterInterface, bool reloadFromNfo)
{
    if (mediaCenterInterface == nullptr) {
        qWarning() << "Passed an empty (null) mediaCenterInterface to loadData";
        return false;
    }
    if ((m_infoLoaded || hasChanged()) && m_infoFromNfoLoaded) {
        return m_infoLoaded;
    }

    const bool infoLoaded = [&]() {
        return reloadFromNfo ? mediaCenterInterface->loadTvShowEpisode(this)
                             : mediaCenterInterface->loadTvShowEpisode(this, nfoContent());
    }();

    if (!infoLoaded && !files().isEmpty()) {
        QStringList filenameParts = files().at(0).split(QDir::separator());
        QString filename = filenameParts.last();
        if (filename.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
            if (filenameParts.count() > 1 && helper::isDvd(files().at(0))) {
                filename = filenameParts.at(filenameParts.count() - 3);
            } else if (filenameParts.count() > 2 && helper::isDvd(files().at(0), true)) {
                filename = filenameParts.at(filenameParts.count() - 2);
            }
        } else if (filename.endsWith("index.bdmv", Qt::CaseInsensitive) && filenameParts.count() > 2) {
            filename = filenameParts.at(filenameParts.count() - 3);
        }
        setName(filename.replace(".", " ").replace("_", " "));
    }
    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    setChanged(false);
    return infoLoaded;
}

/**
 * @brief Load data using a scraper
 * @param id ID of the show for the scraper
 * @param tvScraperInterface ScraperInterface to use
 */
void TvShowEpisode::loadData(TvDbId id, TvScraperInterface* tvScraperInterface, QVector<TvShowScraperInfos> infosToLoad)
{
    qDebug() << "Entered, id=" << id.toString() << "scraperInterface=" << tvScraperInterface->name();
    m_infosToLoad = infosToLoad;
    tvScraperInterface->loadTvShowEpisodeData(id, this, infosToLoad);
}

/**
 * @brief Tries to load streamdetails from the file
 */
void TvShowEpisode::loadStreamDetailsFromFile()
{
    m_streamDetails->loadStreamDetails();
    setStreamDetailsLoaded(true);
    setChanged(true);
}

/**
 * @brief Called from the scraper when loading has finished
 */
void TvShowEpisode::scraperLoadDone()
{
    emit sigLoaded();
}

/**
 * @brief Save data using a MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use
 * @return Saving was successful
 */
bool TvShowEpisode::saveData(MediaCenterInterface* mediaCenterInterface)
{
    qDebug() << "Entered";
    if (!streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails()) {
        loadStreamDetailsFromFile();
    }
    bool saved = mediaCenterInterface->saveTvShowEpisode(this);
    qDebug() << "Saved" << saved;
    if (!m_infoLoaded) {
        m_infoLoaded = saved;
    }
    setSyncNeeded(true);
    setChanged(false);
    clearImages();
    return saved;
}

/**
 * @brief TvShowEpisode::tvShow
 * @return Parent show
 */
TvShow* TvShowEpisode::tvShow() const
{
    return m_parent;
}

bool TvShowEpisode::isValid() const
{
    return !m_files.isEmpty();
}

/**
 * @brief Clears the episode images to save memory
 */
void TvShowEpisode::clearImages()
{
    m_thumbnailImage = QByteArray();
}

/*** GETTER ***/

bool TvShowEpisode::infoLoaded() const
{
    return m_infoLoaded;
}

/**
 * @property TvShow::name
 * @brief Name of the episode
 * @return Name
 * @see TvShow::setName
 */
QString TvShowEpisode::name() const
{
    return m_name;
}

QString TvShowEpisode::completeEpisodeName() const
{
    return QString("S%1E%2 %3").arg(seasonString()).arg(episodeString()).arg(name());
}

QStringList TvShowEpisode::files() const
{
    return m_files;
}

/**
 * @property TvShow::showTitle
 * @brief The title of the parent show
 * @return Title
 */
QString TvShowEpisode::showTitle() const
{
    if (!m_showTitle.isEmpty()) {
        return m_showTitle;
    }
    if (m_parent != nullptr) {
        return m_parent->name();
    }

    return QString();
}

QVector<Rating>& TvShowEpisode::ratings()
{
    return m_ratings;
}

const QVector<Rating>& TvShowEpisode::ratings() const
{
    return m_ratings;
}

double TvShowEpisode::userRating() const
{
    return m_userRating;
}

/**
 * @property TvShowEpisode::season
 * @brief Season number
 * @return Season number
 * @see TvShowEpisode::setSeasonNumber
 */
SeasonNumber TvShowEpisode::season() const
{
    return m_season;
}

/**
 * @property TvShowEpisode::displaySeason
 * @brief Display Season number
 * @return Display Season number
 * @see TvShowEpisode::setDisplaySeasonNumber
 */
SeasonNumber TvShowEpisode::displaySeason() const
{
    return m_displaySeason;
}

/**
 * @brief Season number with leading zero
 * @return Formatted Season number
 * @see TvShowEpisode::season
 */
QString TvShowEpisode::seasonString() const
{
    return season().toPaddedString();
}

/**
 * @property TvShowEpisode::episode
 * @brief Episode number
 * @return Episode number
 * @see TvShowEpisode::setEpisode
 */
EpisodeNumber TvShowEpisode::episode() const
{
    return m_episode;
}

/**
 * @property TvShowEpisode::displayEpisode
 * @brief Display Episode number
 * @return Display Episode number
 * @see TvShowEpisode::setDisplayEpisode
 */
EpisodeNumber TvShowEpisode::displayEpisode() const
{
    return m_displayEpisode;
}

/**
 * @brief Episode number with leading zero
 * @return Formatted episode number
 * @see TvShowEpisode::setEpisode
 */
QString TvShowEpisode::episodeString() const
{
    return episode().toPaddedString();
}

/**
 * @property TvShowEpisode::overview
 * @brief Plot of the episode
 * @return Plot
 * @see TvShowEpisode::setOverview
 */
QString TvShowEpisode::overview() const
{
    return m_overview;
}

/**
 * @property TvShowEpisode::writers
 * @brief Writers of the episode
 * @return List of writers
 * @see TvShowEpisode::setWriters
 */
QStringList TvShowEpisode::writers() const
{
    return m_writers;
}

/**
 * @property TvShowEpisode::directors
 * @brief Directors of the episode
 * @return List of directors
 * @see TvShowEpisode::setDirectors
 */
QStringList TvShowEpisode::directors() const
{
    return m_directors;
}

/**
 * @property TvShowEpisode::playCount
 * @brief Playcount of the episode
 * @return Playcount
 * @see TvShowEpisode::setPlayCount
 */
int TvShowEpisode::playCount() const
{
    return m_playCount;
}

/**
 * @property TvShowEpisode::lastPlayed
 * @brief Holds the last time the episode was played
 * @return Last playtime
 * @see TvShowEpisode::setLastPlayed
 */
QDateTime TvShowEpisode::lastPlayed() const
{
    return m_lastPlayed;
}

/**
 * @property TvShowEpisode::firstAired
 * @brief Holds the first aired date of the episode
 * @return First aired
 * @see TvShowEpisode::setFirstAired
 */
QDate TvShowEpisode::firstAired() const
{
    return m_firstAired;
}

/**
 * @property TvShowEpisode::certification
 * @brief Certification of the episode
 * @return Certification
 * @see TvShowEpisode::setCertification
 */
Certification TvShowEpisode::certification() const
{
    if (m_certification.isValid()) {
        return m_certification;
    }
    if (m_parent != nullptr) {
        return m_parent->certification();
    }

    return Certification::NoCertification;
}

/**
 * @property TvShowEpisode::network
 * @brief Holds the network of the episode
 * @return Network
 * @see TvShowEpisode::setNetwork
 */
QString TvShowEpisode::network() const
{
    if (!m_network.isEmpty()) {
        return m_network;
    }
    if (m_parent != nullptr) {
        return m_parent->network();
    }

    return QString();
}

/**
 * @brief Holds the thumbnail url
 * @return URL of the thumbnail
 */
QUrl TvShowEpisode::thumbnail() const
{
    return m_thumbnail;
}

/**
 * @brief Holds the current thumbnail image
 * @return Image of the thumbnail
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
 * @brief Returns a list of pointer to the writers
 * @return List of pointers
 */
QVector<QString*> TvShowEpisode::writersPointer()
{
    QVector<QString*> writers;
    for (int i = 0, n = m_writers.size(); i < n; ++i) {
        writers.append(&m_writers[i]);
    }
    return writers;
}

/**
 * @brief Returns a list of pointers to the directors
 * @return List of pointers
 */
QVector<QString*> TvShowEpisode::directorsPointer()
{
    QVector<QString*> directors;
    for (int i = 0, n = m_directors.size(); i < n; ++i) {
        directors.append(&m_directors[i]);
    }
    return directors;
}

int TvShowEpisode::episodeId() const
{
    return m_episodeId;
}

/**
 * @property TvShowEpisode::streamDetailsLoaded
 * @brief Holds if the stream details were loaded
 * @return True if the stream details were loaded
 * @see TvShowEpisode::setStreamDetailsLoaded
 */
bool TvShowEpisode::streamDetailsLoaded() const
{
    return m_streamDetailsLoaded;
}

/**
 * @brief The stream details object of this episode
 * @return StreamDetails Object
 */
StreamDetails* TvShowEpisode::streamDetails()
{
    return m_streamDetails;
}

QString TvShowEpisode::nfoContent() const
{
    return m_nfoContent;
}

int TvShowEpisode::databaseId() const
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

/*** SETTER ***/

/**
 * @brief Sets the name
 * @param name Name of the episode
 * @see TvShowEpisode::name
 */
void TvShowEpisode::setName(QString name)
{
    m_name = name;
    setChanged(true);
}

/**
 * @brief Sets the title of the show
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
 * @brief Sets the season
 * @param season Season number
 * @see TvShowEpisode::season
 */
void TvShowEpisode::setSeason(SeasonNumber season)
{
    m_season = season;
    setChanged(true);
}

/**
 * @brief Sets the episode
 * @param episode Episode number
 * @see TvShowEpisode::episode
 */
void TvShowEpisode::setEpisode(EpisodeNumber episode)
{
    m_episode = episode;
    setChanged(true);
}

/**
 * @brief Sets the display season
 * @param season Display Season number
 * @see TvShowEpisode::displaySeason
 */
void TvShowEpisode::setDisplaySeason(SeasonNumber season)
{
    m_displaySeason = season;
    setChanged(true);
}

/**
 * @brief Sets the display episode
 * @param episode Display Episode number
 * @see TvShowEpisode::displayEpisode
 */
void TvShowEpisode::setDisplayEpisode(EpisodeNumber episode)
{
    m_displayEpisode = episode;
    setChanged(true);
}

/**
 * @brief Sets the plot
 * @param overview Plot
 * @see TvShowEpisode::overview
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
 * @brief Sets all writers
 * @param writers List of writers
 * @see TvShowEpisode::writers
 */
void TvShowEpisode::setWriters(QStringList writers)
{
    m_writers = writers;
    setChanged(true);
}

/**
 * @brief Adds a writer
 * @param writer Writer to add
 * @see TvShowEpisode::writers
 */
void TvShowEpisode::addWriter(QString writer)
{
    m_writers.append(writer);
    setChanged(true);
}

/**
 * @brief Adds a director
 * @param director Director to add
 * @see TvShowEpisode::directors
 */
void TvShowEpisode::addDirector(QString director)
{
    m_directors.append(director);
    setChanged(true);
}

/**
 * @brief Sets all directors
 * @param directors List of directors
 * @see TvShowEpisode::directors
 */
void TvShowEpisode::setDirectors(QStringList directors)
{
    m_directors = directors;
    setChanged(true);
}

/**
 * @brief Sets the playcount
 * @param playCount Playcount
 * @see TvShowEpisode::playCount
 */
void TvShowEpisode::setPlayCount(int playCount)
{
    m_playCount = playCount;
    setChanged(true);
}

/**
 * @brief Sets the last playtime
 * @param lastPlayed Last playtime
 * @see TvShowEpisode::lastPlayed
 */
void TvShowEpisode::setLastPlayed(QDateTime lastPlayed)
{
    m_lastPlayed = lastPlayed;
    setChanged(true);
}

/**
 * @brief Set the first aired date
 * @param firstAired Date of first air
 * @see TvShowEpisode::firstAired
 */
void TvShowEpisode::setFirstAired(QDate firstAired)
{
    m_firstAired = firstAired;
    setChanged(true);
}

/**
 * @brief Sets the certification
 * @param certification Certification
 * @see TvShowEpisode::certification
 */
void TvShowEpisode::setCertification(Certification certification)
{
    m_certification = certification;
    setChanged(true);
}

/**
 * @brief Sets the network
 * @param network Name of the network
 * @see TvShowEpisode::network
 */
void TvShowEpisode::setNetwork(QString network)
{
    m_network = network;
    setChanged(true);
}

/**
 * @brief Sets the thumbnail
 * @param url URL of the thumbnail
 * @see TvShowEpisode::thumbnail
 */
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

/**
 * @brief Sets if the stream details were loaded
 * @see TvShowEpisode::streamDetailsLoaded
 */
void TvShowEpisode::setStreamDetailsLoaded(bool loaded)
{
    m_streamDetailsLoaded = loaded;
}

/*** REMOVER ***/

/**
 * @brief Removes a writer
 * @see TvShowEpisode::writers
 */
void TvShowEpisode::removeWriter(QString* writer)
{
    for (int i = 0, n = m_writers.size(); i < n; ++i) {
        if (&m_writers[i] == writer) {
            m_writers.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

/**
 * @brief Removes a director
 * @see TvShowEpisode::directors
 */
void TvShowEpisode::removeDirector(QString* director)
{
    for (int i = 0, n = m_directors.size(); i < n; ++i) {
        if (&m_directors[i] == director) {
            m_directors.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

void TvShowEpisode::setNfoContent(QString content)
{
    m_nfoContent = content;
}

void TvShowEpisode::setDatabaseId(int id)
{
    m_databaseId = id;
}

void TvShowEpisode::setSyncNeeded(bool syncNeeded)
{
    m_syncNeeded = syncNeeded;
}

QVector<ImageType> TvShowEpisode::imagesToRemove() const
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
            m_imagesToRemove.append(type);
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

QVector<const Actor*> TvShowEpisode::actors() const
{
    QVector<const Actor*> actorPtrs;
    for (const auto& actor : m_actors) {
        actorPtrs.push_back(actor.get());
    }
    return actorPtrs;
}

QVector<Actor*> TvShowEpisode::actors()
{
    QVector<Actor*> actorPtrs;
    for (const auto& actor : m_actors) {
        actorPtrs.push_back(actor.get());
    }
    return actorPtrs;
}

void TvShowEpisode::addActor(Actor actor)
{
    if (actor.order == 0 && !m_actors.empty()) {
        actor.order = m_actors.back()->order + 1;
    }
    m_actors.push_back(std::make_unique<Actor>(actor));
    setChanged(true);
}

void TvShowEpisode::removeActor(Actor* actor)
{
    for (int i = 0, n = m_actors.size(); i < n; ++i) {
        if (m_actors[i].get() == actor) {
            m_actors.erase(m_actors.begin() + i);
            break;
        }
    }
    setChanged(true);
}

bool TvShowEpisode::lessThan(TvShowEpisode* a, TvShowEpisode* b)
{
    if (a->season() < b->season()) {
        return true;
    }
    if (a->season() > b->season()) {
        return false;
    }
    if (a->episode() < b->episode()) {
        return true;
    }
    if (a->episode() > b->episode()) {
        return false;
    }

    return (QString::localeAwareCompare(helper::appendArticle(a->name()), helper::appendArticle(b->name())) < 0);
}

ImdbId TvShowEpisode::imdbId() const
{
    return m_imdbId;
}

TvDbId TvShowEpisode::tvdbId() const
{
    return m_tvdbId;
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
    for (const QString& file : episode.files()) {
        out.append(QString("    %1").arg(file).append(nl));
    }
    out.append(QStringLiteral("  Name:          ").append(episode.name()).append(nl));
    out.append(QStringLiteral("  ShowTitle:     ").append(episode.showTitle()).append(nl));
    out.append(QStringLiteral("  Season:        %1").arg(episode.season().toPaddedString()).append(nl));
    out.append(QStringLiteral("  Episode:       %1").arg(episode.episode().toPaddedString()).append(nl));
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
    /*
    for (const QString &studio: movie.studios())
        out.append(QString("  Studio:         ").append(studio)).append(nl);
    for (const QString &country: movie.countries())
        out.append(QString("  Country:       ").append(country)).append(nl);
    for (const Actor &actor: movie.actors()) {
        out.append(QString("  Actor:         ").append(nl));
        out.append(QString("    Name:  ").append(actor.name)).append(nl);
        out.append(QString("    Role:  ").append(actor.role)).append(nl);
        out.append(QString("    Thumb: ").append(actor.thumb)).append(nl);
    }
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

QDebug operator<<(QDebug dbg, const TvShowEpisode* episode)
{
    dbg.nospace() << *episode;
    return dbg.space();
}
