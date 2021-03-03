#include "ConcertController.h"

#include <QDir>
#include <QFileInfo>
#include <QtCore/qmath.h>

#include "concerts/Concert.h"
#include "data/ImageCache.h"
#include "file/NameFormatter.h"
#include "globals/DownloadManager.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "media_centers/MediaCenterInterface.h"
#include "scrapers/concert/ConcertScraper.h"
#include "settings/Settings.h"

ConcertController::ConcertController(Concert* parent) :
    QObject(parent), m_concert{parent}, m_downloadManager{new DownloadManager(this)}
{
    connect(m_downloadManager,
        &DownloadManager::sigDownloadFinished,
        this,
        &ConcertController::onDownloadFinished,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(m_downloadManager,
        &DownloadManager::allConcertDownloadsFinished,
        this,
        &ConcertController::onAllDownloadsFinished,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
}

Concert* ConcertController::concert()
{
    return m_concert;
}

bool ConcertController::saveData(MediaCenterInterface* mediaCenterInterface)
{
    if (!m_concert->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails()) {
        loadStreamDetailsFromFile();
    }
    const bool saved = mediaCenterInterface->saveConcert(m_concert);
    if (!m_infoLoaded) {
        m_infoLoaded = saved;
    }
    m_concert->setChanged(false);
    m_concert->clearImages();
    m_concert->clearExtraFanartData();
    m_concert->setSyncNeeded(true);
    return saved;
}

bool ConcertController::loadData(MediaCenterInterface* mediaCenterInterface, bool force, bool reloadFromNfo)
{
    if ((m_infoLoaded || m_concert->hasChanged()) && !force
        && (m_infoFromNfoLoaded || (m_concert->hasChanged() && !m_infoFromNfoLoaded))) {
        return m_infoLoaded;
    }

    m_concert->blockSignals(true);
    NameFormatter nameFormatter(Settings::instance()->excludeWords());

    bool infoLoaded = false;
    if (reloadFromNfo) {
        infoLoaded = mediaCenterInterface->loadConcert(m_concert);
    } else {
        infoLoaded = mediaCenterInterface->loadConcert(m_concert, m_concert->nfoContent());
    }

    if (!infoLoaded) {
        if (!m_concert->files().isEmpty()) {
            QFileInfo fi(m_concert->files().first().toString());
            if (QString::compare(fi.fileName(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!pathElements.isEmpty()
                    && QString::compare(pathElements.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0) {
                    pathElements.removeLast();
                }
                if (!pathElements.isEmpty()) {
                    m_concert->setTitle(nameFormatter.formatName(pathElements.last()));
                }
            } else if (QString::compare(fi.fileName(), "index.bdmv", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!pathElements.isEmpty()
                    && QString::compare(pathElements.last(), "BDMV", Qt::CaseInsensitive) == 0) {
                    pathElements.removeLast();
                }
                if (!pathElements.isEmpty()) {
                    m_concert->setTitle(nameFormatter.formatName(pathElements.last()));
                }
            } else if (m_concert->inSeparateFolder()) {
                QStringList splitted = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!splitted.isEmpty()) {
                    m_concert->setTitle(nameFormatter.formatName(splitted.last()));
                } else {
                    if (m_concert->files().size() > 1) {
                        m_concert->setTitle(nameFormatter.formatName(nameFormatter.removeParts(fi.completeBaseName())));
                    } else {
                        m_concert->setTitle(nameFormatter.formatName(fi.completeBaseName()));
                    }
                }
            } else {
                if (m_concert->files().size() > 1) {
                    m_concert->setTitle(nameFormatter.formatName(nameFormatter.removeParts(fi.completeBaseName())));
                } else {
                    m_concert->setTitle(nameFormatter.formatName(fi.completeBaseName()));
                }
            }
        }
    }

    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    m_concert->setChanged(false);
    m_concert->blockSignals(false);
    return infoLoaded;
}

void ConcertController::loadData(TmdbId id,
    mediaelch::scraper::ConcertScraper* scraperInterface,
    QSet<ConcertScraperInfo> infos)
{
    m_infosToLoad = infos;
    scraperInterface->loadData(id, m_concert, infos);
}

void ConcertController::loadStreamDetailsFromFile()
{
    using namespace std::chrono;
    m_concert->streamDetails()->loadStreamDetails();
    seconds runtime(
        m_concert->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::DurationInSeconds).toInt());
    m_concert->setRuntime(duration_cast<minutes>(runtime));
    m_concert->setStreamDetailsLoaded(true);
    m_concert->setChanged(true);
}

QSet<ConcertScraperInfo> ConcertController::infosToLoad()
{
    return m_infosToLoad;
}

void ConcertController::setInfosToLoad(QSet<ConcertScraperInfo> infos)
{
    m_infosToLoad = infos;
}

void ConcertController::scraperLoadDone(mediaelch::scraper::ConcertScraper* scraper)
{
    Q_UNUSED(scraper);

    emit sigInfoLoadDone(m_concert);
    if (m_concert->tmdbId().isValid() && infosToLoad().contains(ConcertScraperInfo::ExtraArts)) {
        QVector<ImageType> images{ImageType::ConcertCdArt, ImageType::ConcertClearArt, ImageType::ConcertLogo};
        connect(Manager::instance()->fanartTv(),
            &mediaelch::scraper::ImageProvider::sigConcertImagesLoaded,
            this,
            &ConcertController::onFanartLoadDone,
            Qt::UniqueConnection);
        Manager::instance()->fanartTv()->concertImages(m_concert, m_concert->tmdbId(), images);
    } else {
        onFanartLoadDone(m_concert, QMap<ImageType, QVector<Poster>>());
    }
}

void ConcertController::onFanartLoadDone(Concert* concert, QMap<ImageType, QVector<Poster>> posters)
{
    if (concert != m_concert) {
        return;
    }

    if (infosToLoad().contains(ConcertScraperInfo::Poster) && !m_concert->posters().isEmpty()) {
        posters.insert(ImageType::ConcertPoster, QVector<Poster>() << m_concert->posters().at(0));
    }
    if (infosToLoad().contains(ConcertScraperInfo::Backdrop) && !m_concert->backdrops().isEmpty()) {
        posters.insert(ImageType::ConcertBackdrop, QVector<Poster>() << m_concert->backdrops().at(0));
    }

    QVector<DownloadManagerElement> downloads;

    QVector<ImageType> imageTypes;
    QMapIterator<ImageType, QVector<Poster>> it(posters);
    while (it.hasNext()) {
        it.next();
        if (it.value().isEmpty()) {
            continue;
        }
        DownloadManagerElement d;
        d.imageType = it.key();
        d.url = it.value().at(0).originalUrl;
        d.concert = m_concert;
        downloads.append(d);
        if (!imageTypes.contains(it.key())) {
            imageTypes.append(it.key());
        }
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
        if (elem.imageType == ImageType::ConcertExtraFanart) {
            helper::resizeBackdrop(elem.data);
            m_concert->addExtraFanart(elem.data);
        } else {
            QString filePath = Manager::instance()->mediaCenterInterface()->imageFileName(m_concert, elem.imageType);
            ImageCache::instance()->invalidateImages(mediaelch::FilePath(filePath));
            if (elem.imageType == ImageType::ConcertBackdrop) {
                helper::resizeBackdrop(elem.data);
            }
            m_concert->setImage(elem.imageType, elem.data);
        }
    }

    emit sigImage(m_concert, elem.imageType, elem.data);
}

void ConcertController::loadImage(ImageType type, QUrl url)
{
    DownloadManagerElement d;
    d.concert = m_concert;
    d.imageType = type;
    d.url = url;
    emit sigLoadingImages(m_concert, {type});
    m_downloadManager->addDownload(d);
}

void ConcertController::loadImages(ImageType type, QVector<QUrl> urls)
{
    for (const QUrl& url : urls) {
        DownloadManagerElement d;
        d.concert = m_concert;
        d.imageType = type;
        d.url = url;
        emit sigLoadingImages(m_concert, {type});
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

void ConcertController::setLoadsLeft(QVector<ScraperData> loadsLeft)
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
        scraperLoadDone(nullptr);
    }
    m_loadMutex.unlock();
}
