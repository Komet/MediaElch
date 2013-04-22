#include "MovieController.h"

#include <QDir>
#include <QFileInfo>
#include <QtCore/qmath.h>
#include "data/ImageCache.h"
#include "globals/DownloadManagerElement.h"
#include "globals/Helper.h"
#include "globals/NameFormatter.h"
#include "globals/Manager.h"
#include "settings/Settings.h"

MovieController::MovieController(Movie *parent) :
    QObject(parent)
{
    m_movie = parent;
    m_infoLoaded = false;
    m_infoFromNfoLoaded = false;
    m_downloadManager = new DownloadManager(this);
    m_downloadsInProgress = false;
    m_downloadsSize = 0;

    connect(m_downloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onDownloadFinished(DownloadManagerElement)));
    connect(m_downloadManager, SIGNAL(allDownloadsFinished(Movie*)), this, SLOT(onAllDownloadsFinished()), Qt::UniqueConnection);
}

/**
 * @brief Saves the movies infos with the given MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use for saving
 * @return Saving was successful or not
 */
bool MovieController::saveData(MediaCenterInterface *mediaCenterInterface)
{
    qDebug() << "Entered";

    if (!m_movie->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails())
        loadStreamDetailsFromFile();
    bool saved = mediaCenterInterface->saveMovie(m_movie);
    qDebug() << "Saved" << saved;
    if (!m_infoLoaded)
        m_infoLoaded = saved;
    m_movie->setChanged(false);
    m_movie->clearImages();
    m_movie->clearExtraFanartData();
    m_movie->setSyncNeeded(true);
    return saved;
}

/**
 * @brief Loads the movies infos with the given MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use for loading
 * @param force Force the loading. If set to false and infos were already loeaded this function just returns
 * @return Loading was successful or not
 */
bool MovieController::loadData(MediaCenterInterface *mediaCenterInterface, bool force, bool reloadFromNfo)
{
    if ((m_infoLoaded || m_movie->hasChanged()) && !force && (m_infoFromNfoLoaded || (m_movie->hasChanged() && !m_infoFromNfoLoaded) ))
        return m_infoLoaded;

    m_movie->blockSignals(true);
    NameFormatter *nameFormat = NameFormatter::instance();

    bool infoLoaded;
    if (reloadFromNfo)
        infoLoaded = mediaCenterInterface->loadMovie(m_movie);
    else
        infoLoaded = mediaCenterInterface->loadMovie(m_movie, m_movie->nfoContent());

    if (!infoLoaded) {
        if (m_movie->files().size() > 0) {
            QFileInfo fi(m_movie->files().at(0));
            if (QString::compare(fi.fileName(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (pathElements.size() > 0 && QString::compare(pathElements.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0)
                    pathElements.removeLast();
                if (pathElements.size() > 0)
                    m_movie->setName(nameFormat->formatName(pathElements.last()));
            } else if (QString::compare(fi.fileName(), "index.bdmv", Qt::CaseInsensitive) == 0) {
                    QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                    if (pathElements.size() > 0 && QString::compare(pathElements.last(), "BDMV", Qt::CaseInsensitive) == 0)
                        pathElements.removeLast();
                    if (pathElements.size() > 0)
                        m_movie->setName(nameFormat->formatName(pathElements.last()));
            } else if (m_movie->inSeparateFolder()) {
                QStringList splitted = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!splitted.isEmpty()) {
                    m_movie->setName(nameFormat->formatName(splitted.last()));
                } else {
                    if (m_movie->files().size() > 1)
                        m_movie->setName(nameFormat->formatName(
                                         nameFormat->formatParts(fi.completeBaseName())));
                    else
                        m_movie->setName(nameFormat->formatName(fi.completeBaseName()));
                }
            } else {
                if (m_movie->files().size() > 1)
                    m_movie->setName(nameFormat->formatName(
                                     nameFormat->formatParts(fi.completeBaseName())));
                else
                    m_movie->setName(nameFormat->formatName(fi.completeBaseName()));
            }
        }
    }
    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    m_movie->setChanged(false);
    m_movie->blockSignals(false);
    return infoLoaded;
}

/**
 * @brief Loads the movies info from a scraper
 * @param id Id of the movie within the given ScraperInterface
 * @param scraperInterface ScraperInterface to use for loading
 * @param infos List of infos to load
 */
void MovieController::loadData(QString id, ScraperInterface *scraperInterface, QList<int> infos)
{
    qDebug() << "Entered, id=" << id << "scraperInterface=" << scraperInterface->name();
    m_infosToLoad = infos;
    if (scraperInterface->name() == "The Movie DB")
        m_movie->setTmdbId(id);
    scraperInterface->loadData(id, m_movie, infos);
}

/**
 * @brief Tries to load streamdetails from the file
 */
void MovieController::loadStreamDetailsFromFile()
{
    m_movie->streamDetails()->loadStreamDetails();
    m_movie->setRuntime(qFloor(m_movie->streamDetails()->videoDetails().value("durationinseconds").toInt()/60));
    m_movie->setStreamDetailsLoaded(true);
    m_movie->setChanged(true);
}

/**
 * @brief Movie::infosToLoad
 * @return
 */
QList<int> MovieController::infosToLoad()
{
    return m_infosToLoad;
}

void MovieController::setInfosToLoad(QList<int> infos)
{
    m_infosToLoad = infos;
}

/**
 * @brief Called when a ScraperInterface has finished loading
 *        Emits the loaded signal
 */
void MovieController::scraperLoadDone()
{
    emit sigInfoLoadDone(m_movie);
    if ((!m_movie->tmdbId().isEmpty() || !m_movie->id().isEmpty()) &&
            (infosToLoad().contains(MovieScraperInfos::Logo) || infosToLoad().contains(MovieScraperInfos::ClearArt) || infosToLoad().contains(MovieScraperInfos::CdArt))) {
        QList<int> images;
        if (infosToLoad().contains(MovieScraperInfos::Logo))
            images << TypeLogo;
        if (infosToLoad().contains(MovieScraperInfos::ClearArt))
            images << TypeClearArt;
        if (infosToLoad().contains(MovieScraperInfos::CdArt))
            images << TypeCdArt;
        connect(Manager::instance()->fanartTv(), SIGNAL(sigImagesLoaded(Movie*,QMap<int,QList<Poster> >)), this, SLOT(onFanartLoadDone(Movie*,QMap<int,QList<Poster> >)));
        Manager::instance()->fanartTv()->movieImages(m_movie, (!m_movie->tmdbId().isEmpty()) ? m_movie->tmdbId() : m_movie->id(), images);
    } else {
        onFanartLoadDone(m_movie, QMap<int, QList<Poster> >());
    }
}

void MovieController::onFanartLoadDone(Movie *movie, QMap<int, QList<Poster> > posters)
{
    if (movie != m_movie)
        return;

    if (infosToLoad().contains(MovieScraperInfos::Poster) && !m_movie->posters().isEmpty())
        posters.insert(TypePoster, QList<Poster>() << m_movie->posters().at(0));
    if (infosToLoad().contains(MovieScraperInfos::Backdrop) && !m_movie->backdrops().isEmpty())
        posters.insert(TypeBackdrop, QList<Poster>() << m_movie->backdrops().at(0));

    QList<DownloadManagerElement> downloads;
    if (infosToLoad().contains(MovieScraperInfos::Actors) && Settings::instance()->downloadActorImages()) {
        QList<Actor*> actors = m_movie->actorsPointer();
        for (int i=0, n=actors.size() ; i<n ; i++) {
            if (actors.at(i)->thumb.isEmpty())
                continue;
            DownloadManagerElement d;
            d.imageType = TypeActor;
            d.url = QUrl(actors.at(i)->thumb);
            d.actor = actors.at(i);
            d.movie = movie;
            downloads.append(d);
        }
    }

    QList<int> imageTypes;
    QMapIterator<int, QList<Poster> > it(posters);
    while (it.hasNext()) {
        it.next();
        if (it.value().isEmpty())
            continue;
        DownloadManagerElement d;
        d.imageType = static_cast<ImageType>(it.key());
        d.url = it.value().at(0).originalUrl;
        d.movie = m_movie;
        downloads.append(d);
        if (!imageTypes.contains(it.key()))
            imageTypes.append(it.key());
    }

    if (downloads.isEmpty()) {
        emit sigLoadDone(m_movie);
    } else {
        emit sigLoadingImages(m_movie, imageTypes);
        emit sigLoadImagesStarted(m_movie);
    }

    m_downloadsInProgress = !downloads.isEmpty();
    m_downloadsSize = downloads.count();
    m_downloadsLeft = downloads.count();
    m_downloadManager->setDownloads(downloads);
}

void MovieController::onAllDownloadsFinished()
{
    m_downloadsInProgress = false;
    m_downloadsSize = 0;
    m_downloadsLeft = 0;
    emit sigLoadDone(m_movie);
}

void MovieController::onDownloadFinished(DownloadManagerElement elem)
{
    m_downloadsLeft--;
    emit sigDownloadProgress(m_movie, m_downloadsLeft, m_downloadsSize);

    switch (elem.imageType) {
    case TypePoster:
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->posterImageName(m_movie));
        m_movie->setPosterImage(elem.data);
        break;
    case TypeBackdrop:
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->backdropImageName(m_movie));
        Helper::resizeBackdrop(elem.data);
        m_movie->setBackdropImage(elem.data);
        break;
    case TypeLogo:
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->logoImageName(m_movie));
        m_movie->setLogoImage(elem.data);
        break;
    case TypeClearArt:
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->clearArtImageName(m_movie));
        m_movie->setClearArtImage(elem.data);
        break;
    case TypeCdArt:
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->cdArtImageName(m_movie));
        m_movie->setCdArtImage(elem.data);
        break;
    case TypeActor:
        elem.actor->image = elem.data;
        break;
    case TypeExtraFanart:
        Helper::resizeBackdrop(elem.data);
        m_movie->addExtraFanart(elem.data);
        break;
    default:
        break;
    }

    if (elem.imageType != TypeActor)
        emit sigImage(m_movie, elem.imageType, elem.data);
}

void MovieController::loadImage(int type, QUrl url)
{
    DownloadManagerElement d;
    d.movie = m_movie;
    d.imageType = static_cast<ImageType>(type);
    d.url = url;
    emit sigLoadingImages(m_movie, QList<int>() << type);
    m_downloadManager->addDownload(d);
}

void MovieController::loadImages(int type, QList<QUrl> urls)
{
    foreach (const QUrl &url, urls) {
        DownloadManagerElement d;
        d.movie = m_movie;
        d.imageType = static_cast<ImageType>(type);
        d.url = url;
        emit sigLoadingImages(m_movie, QList<int>() << type);
        m_downloadManager->addDownload(d);
    }
}

/**
 * @brief Holds wether movie infos were loaded from a MediaCenterInterface or ScraperInterface
 * @return Infos were loaded
 */
bool MovieController::infoLoaded() const
{
    return m_infoLoaded;
}

/**
 * @brief Returns true if a download is in progress
 * @return Download is in progress
 */
bool MovieController::downloadsInProgress() const
{
    return m_downloadsInProgress;
}

void MovieController::abortDownloads()
{
    m_downloadManager->abortDownloads();
}

void MovieController::setLoadsLeft(QList<ScraperData> loadsLeft)
{
    m_loadDoneFired = false;
    m_loadsLeft = loadsLeft;
}

void MovieController::removeFromLoadsLeft(ScraperData load)
{
    m_loadsLeft.removeOne(load);
    m_loadMutex.lock();
    if (m_loadsLeft.isEmpty() && !m_loadDoneFired) {
        m_loadDoneFired = true;
        scraperLoadDone();
    }
    m_loadMutex.unlock();
}
