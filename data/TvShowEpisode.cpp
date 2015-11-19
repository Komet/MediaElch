#include "TvShowEpisode.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QTime>
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "settings/Settings.h"

/**
 * @brief TvShowEpisode::TvShowEpisode
 * @param files Files of the episode
 * @param parent
 */
TvShowEpisode::TvShowEpisode(QStringList files, TvShow *parent) :
    QObject(parent)
{
    m_parent = parent;
    m_season = -2;
    m_episode = -2;
    m_displaySeason = -1;
    m_displayEpisode = -1;
    m_playCount = 0;
    m_rating = 0;
    m_votes = 0;
    m_top250 = 0;
    m_thumbnailImageChanged = false;
    m_hasChanged = false;
    static int m_idCounter = 0;
    m_episodeId = ++m_idCounter;
    m_streamDetailsLoaded = false;
    m_infoLoaded = false;
    m_infoFromNfoLoaded = false;
    m_databaseId = -1;
    m_syncNeeded = false;
    m_isDummy = false;
    setFiles(files);
}

void TvShowEpisode::setFiles(QStringList files)
{
    m_files = files;
    if (!files.isEmpty())
        m_streamDetails = new StreamDetails(this, files);
    else
        m_streamDetails = new StreamDetails(this, QStringList());
}

/**
 * @brief TvShowEpisode::setShow
 * @param show
 */
void TvShowEpisode::setShow(TvShow *show)
{
    m_parent = show;
    setParent(show);
}

/**
 * @brief Clears the episodes data
 */
void TvShowEpisode::clear()
{
    QList<int> infos;
    infos << TvShowScraperInfos::Certification
          << TvShowScraperInfos::Rating
          << TvShowScraperInfos::Director
          << TvShowScraperInfos::Writer
          << TvShowScraperInfos::Overview
          << TvShowScraperInfos::Network
          << TvShowScraperInfos::FirstAired
          << TvShowScraperInfos::Thumbnail
          << TvShowScraperInfos::Actors;
    clear(infos);
    m_nfoContent.clear();
}

void TvShowEpisode::clear(QList<int> infos)
{
    if (infos.contains(TvShowScraperInfos::Certification))
        m_certification = "";
    if (infos.contains(TvShowScraperInfos::Rating)) {
        m_rating = 0;
        m_votes = 0;
        m_top250 = 0;
    }
    if (infos.contains(TvShowScraperInfos::Director))
        m_directors.clear();
    if (infos.contains(TvShowScraperInfos::Writer))
        m_writers.clear();
    if (infos.contains(TvShowScraperInfos::Overview))
        m_overview = "";
    if (infos.contains(TvShowScraperInfos::Network))
        m_network = "";
    if (infos.contains(TvShowScraperInfos::FirstAired))
        m_firstAired = QDate(2000, 02, 30); // invalid date;
    if (infos.contains(TvShowScraperInfos::Thumbnail)) {
        m_thumbnail = QUrl();
        m_thumbnailImageChanged = false;
        m_imagesToRemove.removeOne(ImageType::TvShowEpisodeThumb);
    }
    if (infos.contains(TvShowScraperInfos::Actors))
        m_actors.clear();

    m_hasChanged = false;
}

QList<int> TvShowEpisode::infosToLoad()
{
    return m_infosToLoad;
}

/**
 * @brief Load data using a MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use
 * @return Loading was successful
 */
bool TvShowEpisode::loadData(MediaCenterInterface *mediaCenterInterface, bool reloadFromNfo)
{
    if ((m_infoLoaded || hasChanged()) && m_infoFromNfoLoaded)
        return m_infoLoaded;

    bool infoLoaded;
    if (reloadFromNfo)
        infoLoaded = mediaCenterInterface->loadTvShowEpisode(this);
    else
        infoLoaded = mediaCenterInterface->loadTvShowEpisode(this, nfoContent());


    if (!infoLoaded) {
        if (files().count() > 0) {
            QStringList filenameParts = files().at(0).split(QDir::separator());
            QString filename = filenameParts.last();
            if (filename.endsWith("VIDEO_TS.IFO", Qt::CaseInsensitive)) {
                if (filenameParts.count() > 1 && Helper::instance()->isDvd(files().at(0)))
                    filename = filenameParts.at(filenameParts.count()-3);
                else if (filenameParts.count() > 2 && Helper::instance()->isDvd(files().at(0), true))
                    filename = filenameParts.at(filenameParts.count()-2);
            } else if (filename.endsWith("index.bdmv", Qt::CaseInsensitive)) {
                if (filenameParts.count() > 2)
                    filename = filenameParts.at(filenameParts.count()-3);
            }
            setName(filename.replace(".", " ").replace("_", " "));
        }
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
void TvShowEpisode::loadData(QString id, TvScraperInterface *tvScraperInterface, QList<int> infosToLoad)
{
    qDebug() << "Entered, id=" << id << "scraperInterface=" << tvScraperInterface->name();
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
bool TvShowEpisode::saveData(MediaCenterInterface *mediaCenterInterface)
{
    qDebug() << "Entered";
    if (!streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails())
        loadStreamDetailsFromFile();
    bool saved = mediaCenterInterface->saveTvShowEpisode(this);
    qDebug() << "Saved" << saved;
    if (!m_infoLoaded)
        m_infoLoaded = saved;
    setChanged(false);
    setSyncNeeded(true);
    clearImages();
    return saved;
}

/**
 * @brief TvShowEpisode::tvShow
 * @return Parent show
 */
TvShow *TvShowEpisode::tvShow()
{
    return m_parent;
}

/**
 * @brief TvShowEpisode::isValid
 * @return
 */
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

/**
 * @brief TvShowEpisode::infoLoaded
 * @return
 */
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

/**
 * @brief TvShowEpisode::completeEpisodeName
 * @return
 */
QString TvShowEpisode::completeEpisodeName() const
{
    return QString("S%1E%2 %3").arg(seasonString())
                               .arg(episodeString())
                               .arg(name());
}

/**
 * @brief TvShowEpisode::files
 * @return
 */
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
    if (!m_showTitle.isEmpty())
        return m_showTitle;
    if (m_parent)
        return m_parent->name();

    return QString();
}

/**
 * @property TvShowEpisode::rating
 * @brief Rating of the episode
 * @return Rating
 * @see TvShowEpisode::setRating
 */
qreal TvShowEpisode::rating() const
{
    return m_rating;
}

/**
 * @property TvShowEpisode::season
 * @brief Season number
 * @return Season number
 * @see TvShowEpisode::setSeasonNumber
 */
int TvShowEpisode::season() const
{
    return m_season;
}

/**
 * @property TvShowEpisode::displaySeason
 * @brief Display Season number
 * @return Display Season number
 * @see TvShowEpisode::setDisplaySeasonNumber
 */
int TvShowEpisode::displaySeason() const
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
    if (season() == -2)
        return QString("xx");
    return QString("%1").arg(season()).prepend((season() < 10) ? "0" : "");
}

/**
 * @property TvShowEpisode::episode
 * @brief Episode number
 * @return Episode number
 * @see TvShowEpisode::setEpisode
 */
int TvShowEpisode::episode() const
{
    return m_episode;
}

/**
 * @property TvShowEpisode::displayEpisode
 * @brief Display Episode number
 * @return Display Episode number
 * @see TvShowEpisode::setDisplayEpisode
 */
int TvShowEpisode::displayEpisode() const
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
    if (episode() == -2)
        return QString("xx");
    return QString("%1").arg(episode()).prepend((episode() < 10) ? "0" : "");
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
QString TvShowEpisode::certification() const
{
    if (!m_certification.isEmpty())
        return m_certification;
    if (m_parent)
        return m_parent->certification();

    return QString();
}

/**
 * @property TvShowEpisode::network
 * @brief Holds the network of the episode
 * @return Network
 * @see TvShowEpisode::setNetwork
 */
QString TvShowEpisode::network() const
{
    if (!m_network.isEmpty())
        return m_network;
    if (m_parent)
        return m_parent->network();

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

/**
 * @brief TvShowEpisode::thumbnailImageChanged
 * @return
 */
bool TvShowEpisode::thumbnailImageChanged() const
{
    return m_thumbnailImageChanged;
}

/**
 * @brief TvShowEpisode::modelItem
 * @return
 */
TvShowModelItem *TvShowEpisode::modelItem()
{
    return m_modelItem;
}

/**
 * @brief TvShowEpisode::hasChanged
 * @return
 */
bool TvShowEpisode::hasChanged() const
{
    return m_hasChanged;
}

/**
 * @brief Returns a list of pointer to the writers
 * @return List of pointers
 */
QList<QString*> TvShowEpisode::writersPointer()
{
    QList<QString*> writers;
    for (int i=0, n=m_writers.size() ; i<n ; ++i)
        writers.append(&m_writers[i]);
    return writers;
}

/**
 * @brief Returns a list of pointers to the directors
 * @return List of pointers
 */
QList<QString*> TvShowEpisode::directorsPointer()
{
    QList<QString*> directors;
    for (int i=0, n=m_directors.size() ; i<n ; ++i)
        directors.append(&m_directors[i]);
    return directors;
}

/**
 * @brief TvShowEpisode::episodeId
 * @return
 */
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
StreamDetails *TvShowEpisode::streamDetails()
{
    return m_streamDetails;
}

/**
 * @brief TvShowEpisode::nfoContent
 * @return
 */
QString TvShowEpisode::nfoContent() const
{
    return m_nfoContent;
}

/**
 * @brief TvShowEpisode::databaseId
 * @return
 */
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
 * @param showTitle
 */
void TvShowEpisode::setShowTitle(QString showTitle)
{
    m_showTitle = showTitle;
    setChanged(true);
}

/**
 * @brief Sets the rating
 * @param rating Rating
 * @see TvShowEpisode::rating
 */
void TvShowEpisode::setRating(qreal rating)
{
    m_rating = rating;
    setChanged(true);
}

/**
 * @brief Sets the season
 * @param season Season number
 * @see TvShowEpisode::season
 */
void TvShowEpisode::setSeason(int season)
{
    m_season = season;
    setChanged(true);
}

/**
 * @brief Sets the episode
 * @param episode Episode number
 * @see TvShowEpisode::episode
 */
void TvShowEpisode::setEpisode(int episode)
{
    m_episode = episode;
    setChanged(true);
}

/**
 * @brief Sets the display season
 * @param season Display Season number
 * @see TvShowEpisode::displaySeason
 */
void TvShowEpisode::setDisplaySeason(int season)
{
    m_displaySeason = season;
    setChanged(true);
}

/**
 * @brief Sets the display episode
 * @param episode Display Episode number
 * @see TvShowEpisode::displayEpisode
 */
void TvShowEpisode::setDisplayEpisode(int episode)
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
void TvShowEpisode::setCertification(QString certification)
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

/**
 * @brief TvShowEpisode::setThumbnailImage
 * @param thumbnail
 */
void TvShowEpisode::setThumbnailImage(QByteArray thumbnail)
{
    m_thumbnailImage = thumbnail;
    m_thumbnailImageChanged = true;
    setChanged(true);
}

/**
 * @brief TvShowEpisode::setInfosLoaded
 * @param loaded
 */
void TvShowEpisode::setInfosLoaded(bool loaded)
{
    m_infoLoaded = loaded;
}

/**
 * @brief TvShowEpisode::setChanged
 * @param changed
 */
void TvShowEpisode::setChanged(bool changed)
{
    m_hasChanged = changed;
    emit sigChanged(this);
}

/**
 * @brief TvShowEpisode::setModelItem
 * @param item
 */
void TvShowEpisode::setModelItem(TvShowModelItem *item)
{
    m_modelItem = item;
}

/**
 * @brief Sets if the stream details were loaded
 * @param loaded
 * @see TvShowEpisode::streamDetailsLoaded
 */
void TvShowEpisode::setStreamDetailsLoaded(bool loaded)
{
    m_streamDetailsLoaded = loaded;
}

/*** REMOVER ***/

/**
 * @brief Removes a writer
 * @param writer
 * @see TvShowEpisode::writers
 */
void TvShowEpisode::removeWriter(QString *writer)
{
    for (int i=0, n=m_writers.size() ; i<n ; ++i) {
        if (&m_writers[i] == writer) {
            m_writers.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

/**
 * @brief Removes a director
 * @param director
 * @see TvShowEpisode::directors
 */
void TvShowEpisode::removeDirector(QString *director)
{
    for (int i=0, n=m_directors.size() ; i<n ; ++i) {
        if (&m_directors[i] == director) {
            m_directors.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

/**
 * @brief TvShowEpisode::setNfoContent
 * @param content
 */
void TvShowEpisode::setNfoContent(QString content)
{
    m_nfoContent = content;
}

/**
 * @brief TvShowEpisode::setDatabaseId
 * @param id
 */
void TvShowEpisode::setDatabaseId(int id)
{
    m_databaseId = id;
}

void TvShowEpisode::setSyncNeeded(bool syncNeeded)
{
    m_syncNeeded = syncNeeded;
}

QList<int> TvShowEpisode::imagesToRemove() const
{
    return m_imagesToRemove;
}

void TvShowEpisode::removeImage(int type)
{
    switch (type) {
    case ImageType::TvShowEpisodeThumb:
        if (!m_thumbnailImage.isNull()) {
            m_thumbnailImage = QByteArray();
            m_thumbnailImageChanged = false;
        } else if (!m_imagesToRemove.contains(type)) {
            m_imagesToRemove.append(type);
        }
        break;
    default:
        break;
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

QList<Actor> TvShowEpisode::actors() const
{
    return m_actors;
}

QList<Actor*> TvShowEpisode::actorsPointer()
{
    QList<Actor*> actors;
    for (int i=0, n=m_actors.size() ; i<n ; i++)
        actors.append(&(m_actors[i]));
    return actors;
}

void TvShowEpisode::addActor(Actor actor)
{
    m_actors.append(actor);
    setChanged(true);
}

void TvShowEpisode::removeActor(Actor *actor)
{
    for (int i=0, n=m_actors.size() ; i<n ; ++i) {
        if (&m_actors[i] == actor) {
            m_actors.removeAt(i);
            break;
        }
    }
    setChanged(true);
}

bool TvShowEpisode::lessThan(TvShowEpisode *a, TvShowEpisode *b)
{
    if (a->season() < b->season())
        return true;
    if (a->season() > b->season())
        return false;
    if (a->episode() < b->episode())
        return true;
    if (a->episode() > b->episode())
        return false;

    return (QString::localeAwareCompare(Helper::instance()->appendArticle(a->name()), Helper::instance()->appendArticle(b->name())) < 0);
}

QString TvShowEpisode::imdbId() const
{
    return m_imdbId;
}

void TvShowEpisode::setImdbId(const QString &imdbId)
{
    m_imdbId = imdbId;
    setChanged(true);
}

int TvShowEpisode::votes() const
{
    return m_votes;
}

void TvShowEpisode::setVotes(int votes)
{
    m_votes = votes;
    setChanged(true);
}

int TvShowEpisode::top250() const
{
    return m_top250;
}

void TvShowEpisode::setTop250(int top250)
{
    m_top250 = top250;
    setChanged(true);
}

/*** DEBUG ***/

QDebug operator<<(QDebug dbg, const TvShowEpisode &episode)
{
    QString nl = "\n";
    QString out;
    out.append("TvShowEpisode").append(nl);
    out.append(QString("  Files:         ").append(nl));
    foreach (const QString &file, episode.files())
        out.append(QString("    %1").arg(file).append(nl));
    out.append(QString("  Name:          ").append(episode.name()).append(nl));
    out.append(QString("  ShowTitle:     ").append(episode.showTitle()).append(nl));
    out.append(QString("  Season:        %1").arg(episode.season()).append(nl));
    out.append(QString("  Episode:       %1").arg(episode.episode()).append(nl));
    out.append(QString("  Rating:        %1").arg(episode.rating()).append(nl));
    out.append(QString("  FirstAired:    ").append(episode.firstAired().toString("yyyy-MM-dd")).append(nl));
    out.append(QString("  LastPlayed:    ").append(episode.lastPlayed().toString("yyyy-MM-dd")).append(nl));
    out.append(QString("  Playcount:     %1%2").arg(episode.playCount()).arg(nl));
    out.append(QString("  Certification: ").append(episode.certification()).append(nl));
    out.append(QString("  Overview:      ").append(episode.overview())).append(nl);
    foreach (const QString &writer, episode.writers())
        out.append(QString("  Writer:        ").append(writer)).append(nl);
    foreach (const QString &director, episode.directors())
        out.append(QString("  Director:      ").append(director)).append(nl);
    /*
    foreach (const QString &studio, movie.studios())
        out.append(QString("  Studio:         ").append(studio)).append(nl);
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
    */
    dbg.nospace() << out;
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const TvShowEpisode *episode)
{
    dbg.nospace() << *episode;
    return dbg.space();
}
