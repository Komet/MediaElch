#include "AlbumController.h"

#include "data/ImageCache.h"
#include "globals/DownloadManager.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "music/Album.h"
#include "scrapers/image/FanartTvMusic.h"
#include "scrapers/music/MusicScraper.h"

#include <QFileInfo>

AlbumController::AlbumController(Album* parent) :
    QObject(parent),
    m_album{parent},
    m_infoLoaded{false},
    m_infoFromNfoLoaded{false},
    m_downloadManager{new DownloadManager(this)}
{
    connect(m_downloadManager,
        &DownloadManager::sigDownloadFinished,
        this,
        &AlbumController::onDownloadFinished,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(m_downloadManager,
        &DownloadManager::allAlbumDownloadsFinished,
        this,
        &AlbumController::onAllDownloadsFinished,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
}

AlbumController::~AlbumController() = default;

bool AlbumController::loadData(MediaCenterInterface* mediaCenterInterface, bool force, bool reloadFromNfo)
{
    if ((m_infoLoaded || m_album->hasChanged()) && !force
        && (m_infoFromNfoLoaded || (m_album->hasChanged() && !m_infoFromNfoLoaded))) {
        return m_infoLoaded;
    }

    m_album->blockSignals(true);

    bool infoLoaded = false;
    if (reloadFromNfo) {
        infoLoaded = mediaCenterInterface->loadAlbum(m_album);
    } else {
        infoLoaded = mediaCenterInterface->loadAlbum(m_album, m_album->nfoContent());
    }

    if (!infoLoaded) {
        QFileInfo fi(m_album->path().toString());
        m_album->setTitle(fi.fileName());
    }
    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    m_album->setHasChanged(false);
    m_album->blockSignals(false);
    return infoLoaded;
}

bool AlbumController::saveData(MediaCenterInterface* mediaCenterInterface)
{
    bool saved = mediaCenterInterface->saveAlbum(m_album);
    if (!m_infoLoaded) {
        m_infoLoaded = saved;
    }
    m_album->setHasChanged(false);
    m_album->clearImages();
    m_album->bookletModel()->clear();
    if (saved) {
        emit sigSaved(m_album);
    }
    return saved;
}

bool AlbumController::infoLoaded() const
{
    return m_infoLoaded;
}

void AlbumController::setInfoLoaded(bool infoLoaded)
{
    m_infoLoaded = infoLoaded;
}

bool AlbumController::infoFromNfoLoaded() const
{
    return m_infoFromNfoLoaded;
}

void AlbumController::setInfoFromNfoLoaded(bool infoFromNfoLoaded)
{
    m_infoFromNfoLoaded = infoFromNfoLoaded;
}

void AlbumController::loadImage(ImageType type, QUrl url)
{
    DownloadManagerElement d;
    d.album = m_album;
    d.imageType = type;
    d.url = url;
    emit sigLoadingImages(m_album, {type});
    m_downloadManager->addDownload(d);
}

void AlbumController::loadImages(ImageType type, QVector<QUrl> urls)
{
    bool started = false;
    for (const QUrl& url : urls) {
        DownloadManagerElement d;
        d.album = m_album;
        d.imageType = type;
        d.url = url;
        if (!started) {
            emit sigLoadingImages(m_album, {type});
            started = true;
        }
        m_downloadManager->addDownload(d);
    }
}

void AlbumController::onAllDownloadsFinished()
{
    m_downloadsInProgress = false;
    m_downloadsSize = 0;
    m_downloadsLeft = 0;
    emit sigLoadDone(m_album);
}

void AlbumController::onDownloadFinished(DownloadManagerElement elem)
{
    m_downloadsLeft--;
    emit sigDownloadProgress(m_album, m_downloadsLeft, m_downloadsSize);

    if (!elem.data.isEmpty() && elem.imageType == ImageType::AlbumBooklet) {
        auto* image = new Image;
        image->setRawData(elem.data);
        m_album->bookletModel()->addImage(image);
    } else if (!elem.data.isEmpty()) {
        ImageCache::instance()->invalidateImages(
            Manager::instance()->mediaCenterInterface()->imageFileName(m_album, elem.imageType));
        m_album->setRawImage(elem.imageType, elem.data);
    }

    emit sigImage(m_album, elem.imageType, elem.data);
}

bool AlbumController::downloadsInProgress() const
{
    return m_downloadsInProgress;
}

void AlbumController::loadData(MusicBrainzId id,
    MusicBrainzId id2,
    mediaelch::scraper::MusicScraper* scraperInterface,
    QSet<MusicScraperInfo> infos)
{
    m_infosToLoad = infos;
    scraperInterface->loadData(id, id2, m_album, infos);
}

void AlbumController::scraperLoadDone(mediaelch::scraper::MusicScraper* scraper)
{
    emit sigInfoLoadDone(m_album);

    if (scraper == nullptr) {
        onFanartLoadDone(m_album, QMap<ImageType, QVector<Poster>>());
        return;
    }

    QVector<ImageType> images;
    if (m_infosToLoad.contains(MusicScraperInfo::Cover)) {
        images << ImageType::AlbumThumb;
        m_album->clear({MusicScraperInfo::Cover});
    }
    if (m_infosToLoad.contains(MusicScraperInfo::CdArt)) {
        images << ImageType::AlbumCdArt;
        m_album->clear({MusicScraperInfo::CdArt});
    }

    if (!images.isEmpty() && m_album->mbReleaseGroupId().isValid()) {
        mediaelch::scraper::ImageProvider* imageProvider = nullptr;
        for (auto* interface : Manager::instance()->imageProviders()) {
            if (interface->meta().identifier == mediaelch::scraper::FanartTvMusic::ID) {
                imageProvider = interface;
                break;
            }
        }
        if (imageProvider == nullptr) {
            onFanartLoadDone(m_album, QMap<ImageType, QVector<Poster>>());
            return;
        }
        connect(imageProvider,
            &mediaelch::scraper::ImageProvider::sigAlbumImagesLoaded,
            this,
            &AlbumController::onFanartLoadDone,
            Qt::UniqueConnection);
        imageProvider->albumImages(m_album, m_album->mbReleaseGroupId(), images);
    } else {
        onFanartLoadDone(m_album, QMap<ImageType, QVector<Poster>>());
    }
}

void AlbumController::onFanartLoadDone(Album* album, QMap<ImageType, QVector<Poster>> posters)
{
    if (album != m_album) {
        return;
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
        d.album = m_album;
        downloads.append(d);
        if (!imageTypes.contains(it.key())) {
            imageTypes.append(it.key());
        }
    }

    if (downloads.isEmpty()) {
        emit sigLoadDone(m_album);
    } else {
        emit sigLoadingImages(m_album, imageTypes);
        emit sigLoadImagesStarted(m_album);
    }

    m_downloadsInProgress = !downloads.isEmpty();
    m_downloadsSize = downloads.count();
    m_downloadsLeft = downloads.count();
    m_downloadManager->setDownloads(downloads);
}

void AlbumController::abortDownloads()
{
    m_downloadManager->abortDownloads();
}
