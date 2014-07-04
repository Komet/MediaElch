#include "ConcertController.h"

#include <QDir>
#include <QFileInfo>
#include <QtCore/qmath.h>
#include "data/ImageCache.h"
#include "globals/DownloadManagerElement.h"
#include "globals/Helper.h"
#include "globals/NameFormatter.h"
#include "globals/Manager.h"
#include "settings/Settings.h"

ConcertController::ConcertController(Concert *parent) :
    QObject(parent)
{
    m_concert = parent;
    m_infoLoaded = false;
    m_infoFromNfoLoaded = false;
    m_downloadManager = new DownloadManager(this);
    m_downloadsInProgress = false;
    m_downloadsSize = 0;

    connect(m_downloadManager, SIGNAL(downloadFinished(DownloadManagerElement)), this, SLOT(onDownloadFinished(DownloadManagerElement)));
    connect(m_downloadManager, SIGNAL(allDownloadsFinished(Concert*)), this, SLOT(onAllDownloadsFinished()), Qt::UniqueConnection);
}

bool ConcertController::saveData(MediaCenterInterface *mediaCenterInterface)
{
    if (!m_concert->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails())
        loadStreamDetailsFromFile();
    bool saved = mediaCenterInterface->saveConcert(m_concert);
    if (!m_infoLoaded)
        m_infoLoaded = saved;
    m_concert->setChanged(false);
    m_concert->clearImages();
    m_concert->clearExtraFanartData();
    m_concert->setSyncNeeded(true);
    return saved;
}

bool ConcertController::loadData(MediaCenterInterface *mediaCenterInterface, bool force, bool reloadFromNfo)
{
    if ((m_infoLoaded || m_concert->hasChanged()) && !force && (m_infoFromNfoLoaded || (m_concert->hasChanged() && !m_infoFromNfoLoaded) ))
        return m_infoLoaded;

    m_concert->blockSignals(true);
    NameFormatter *nameFormat = NameFormatter::instance();

    bool infoLoaded;
    if (reloadFromNfo)
        infoLoaded = mediaCenterInterface->loadConcert(m_concert);
    else
        infoLoaded = mediaCenterInterface->loadConcert(m_concert, m_concert->nfoContent());

    if (!infoLoaded) {
        if (m_concert->files().size() > 0) {
            QFileInfo fi(m_concert->files().at(0));
            if (QString::compare(fi.fileName(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (pathElements.size() > 0 && QString::compare(pathElements.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0)
                    pathElements.removeLast();
                if (pathElements.size() > 0)
                    m_concert->setName(nameFormat->formatName(pathElements.last()));
            } else if (QString::compare(fi.fileName(), "index.bdmv", Qt::CaseInsensitive) == 0) {
                    QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                    if (pathElements.size() > 0 && QString::compare(pathElements.last(), "BDMV", Qt::CaseInsensitive) == 0)
                        pathElements.removeLast();
                    if (pathElements.size() > 0)
                        m_concert->setName(nameFormat->formatName(pathElements.last()));
            } else if (m_concert->inSeparateFolder()) {
                QStringList splitted = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!splitted.isEmpty()) {
                    m_concert->setName(nameFormat->formatName(splitted.last()));
                } else {
                    if (m_concert->files().size() > 1)
                        m_concert->setName(nameFormat->formatName(nameFormat->formatParts(fi.completeBaseName())));
                    else
                        m_concert->setName(nameFormat->formatName(fi.completeBaseName()));
                }
            } else {
                if (m_concert->files().size() > 1)
                    m_concert->setName(nameFormat->formatName(nameFormat->formatParts(fi.completeBaseName())));
                else
                    m_concert->setName(nameFormat->formatName(fi.completeBaseName()));
            }


        }
    }

    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    m_concert->setChanged(false);
    m_concert->blockSignals(false);
    return infoLoaded;
}

void ConcertController::loadData(QString id, ConcertScraperInterface *scraperInterface, QList<int> infos)
{
    m_infosToLoad = infos;
    scraperInterface->loadData(id, m_concert, infos);
}

void ConcertController::loadStreamDetailsFromFile()
{
    m_concert->streamDetails()->loadStreamDetails();
    m_concert->setRuntime(qFloor(m_concert->streamDetails()->videoDetails().value("durationinseconds").toInt()/60));
    m_concert->setStreamDetailsLoaded(true);
    m_concert->setChanged(true);
}

QList<int> ConcertController::infosToLoad()
{
    return m_infosToLoad;
}

void ConcertController::setInfosToLoad(QList<int> infos)
{
    m_infosToLoad = infos;
}

void ConcertController::scraperLoadDone(ConcertScraperInterface *scraper)
{
    Q_UNUSED(scraper);

    emit sigInfoLoadDone(m_concert);
    if (!m_concert->tmdbId().isEmpty() && infosToLoad().contains(ConcertScraperInfos::ExtraArts)) {
        QList<int> images;
        images << ImageType::ConcertCdArt
               << ImageType::ConcertClearArt
               << ImageType::ConcertLogo;
        connect(Manager::instance()->fanartTv(), SIGNAL(sigImagesLoaded(Concert*,QMap<int,QList<Poster> >)), this, SLOT(onFanartLoadDone(Concert*,QMap<int,QList<Poster> >)), Qt::UniqueConnection);
        Manager::instance()->fanartTv()->concertImages(m_concert, m_concert->tmdbId(), images);
    } else {
        onFanartLoadDone(m_concert, QMap<int, QList<Poster> >());
    }
}

void ConcertController::onFanartLoadDone(Concert *concert, QMap<int, QList<Poster> > posters)
{
    if (concert != m_concert)
        return;

    if (infosToLoad().contains(ConcertScraperInfos::Poster) && !m_concert->posters().isEmpty())
        posters.insert(ImageType::ConcertPoster, QList<Poster>() << m_concert->posters().at(0));
    if (infosToLoad().contains(ConcertScraperInfos::Backdrop) && !m_concert->backdrops().isEmpty())
        posters.insert(ImageType::ConcertBackdrop, QList<Poster>() << m_concert->backdrops().at(0));

    QList<DownloadManagerElement> downloads;

    QList<int> imageTypes;
    QMapIterator<int, QList<Poster> > it(posters);
    while (it.hasNext()) {
        it.next();
        if (it.value().isEmpty())
            continue;
        DownloadManagerElement d;
        d.imageType = it.key();
        d.url = it.value().at(0).originalUrl;
        d.concert = m_concert;
        downloads.append(d);
        if (!imageTypes.contains(it.key()))
            imageTypes.append(it.key());
    }

    if (downloads.isEmpty()) {
        emit sigLoadDone(m_concert);
    } else {
        emit sigLoadingImages(m_concert, imageTypes);
        emit sigLoadImagesStarted(m_concert);
    }

    m_downloadsInProgress = !downloads.isEmpty();
    m_downloadsSize = downloads.count();
    m_downloadsLeft = downloads.count();
    m_downloadManager->setDownloads(downloads);
}

void ConcertController::onAllDownloadsFinished()
{
    m_downloadsInProgress = false;
    m_downloadsSize = 0;
    m_downloadsLeft = 0;
    emit sigLoadDone(m_concert);
}

void ConcertController::onDownloadFinished(DownloadManagerElement elem)
{
    m_downloadsLeft--;
    emit sigDownloadProgress(m_concert, m_downloadsLeft, m_downloadsSize);

    if (!elem.data.isEmpty()) {
        ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(m_concert, elem.imageType));
        if (elem.imageType == ImageType::ConcertBackdrop)
            Helper::instance()->resizeBackdrop(elem.data);
        m_concert->setImage(elem.imageType, elem.data);
    }

    emit sigImage(m_concert, elem.imageType, elem.data);
}

void ConcertController::loadImage(int type, QUrl url)
{
    DownloadManagerElement d;
    d.concert = m_concert;
    d.imageType = type;
    d.url = url;
    emit sigLoadingImages(m_concert, QList<int>() << type);
    m_downloadManager->addDownload(d);
}

void ConcertController::loadImages(int type, QList<QUrl> urls)
{
    foreach (const QUrl &url, urls) {
        DownloadManagerElement d;
        d.concert = m_concert;
        d.imageType = type;
        d.url = url;
        emit sigLoadingImages(m_concert, QList<int>() << type);
        m_downloadManager->addDownload(d);
    }
}

bool ConcertController::infoLoaded() const
{
    return m_infoLoaded;
}

bool ConcertController::downloadsInProgress() const
{
    return m_downloadsInProgress;
}

void ConcertController::abortDownloads()
{
    m_downloadManager->abortDownloads();
}

void ConcertController::setLoadsLeft(QList<ScraperData> loadsLeft)
{
    m_loadDoneFired = false;
    m_loadsLeft = loadsLeft;
}

void ConcertController::removeFromLoadsLeft(ScraperData load)
{
    m_loadsLeft.removeOne(load);
    m_loadMutex.lock();
    if (m_loadsLeft.isEmpty() && !m_loadDoneFired) {
        m_loadDoneFired = true;
        scraperLoadDone(0);
    }
    m_loadMutex.unlock();
}
