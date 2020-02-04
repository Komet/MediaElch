#include "ArtistController.h"

#include "data/ImageCache.h"
#include "globals/DownloadManager.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "media_centers/MediaCenterInterface.h"
#include "music/Artist.h"
#include "scrapers/music/MusicScraperInterface.h"

#include <QDebug>
#include <QFileInfo>

ArtistController::ArtistController(Artist* parent) :
    QObject(parent),
    m_artist{parent},
    m_infoLoaded{false},
    m_infoFromNfoLoaded{false},
    m_downloadManager{new DownloadManager(this)},
    m_downloadsInProgress{false},
    m_downloadsSize{0}
{
    connect(m_downloadManager, &DownloadManager::sigDownloadFinished, this, &ArtistController::onDownloadFinished);
    connect(m_downloadManager,
        SIGNAL(allDownloadsFinished(Artist*)),
        this,
        SLOT(onAllDownloadsFinished()),
        Qt::UniqueConnection);
}

bool ArtistController::loadData(MediaCenterInterface* mediaCenterInterface, bool force, bool reloadFromNfo)
{
    if ((m_infoLoaded || m_artist->hasChanged()) && !force
        && (m_infoFromNfoLoaded || (m_artist->hasChanged() && !m_infoFromNfoLoaded))) {
        return m_infoLoaded;
    }

    m_artist->blockSignals(true);

    bool infoLoaded;
    if (reloadFromNfo) {
        infoLoaded = mediaCenterInterface->loadArtist(m_artist);
    } else {
        infoLoaded = mediaCenterInterface->loadArtist(m_artist, m_artist->nfoContent());
    }

    if (!infoLoaded) {
        QFileInfo fi(m_artist->path());
        m_artist->setName(fi.fileName());
    }
    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    m_artist->setHasChanged(false);
    m_artist->blockSignals(false);
    return infoLoaded;
}

bool ArtistController::saveData(MediaCenterInterface* mediaCenterInterface)
{
    bool saved = mediaCenterInterface->saveArtist(m_artist);
    if (!m_infoLoaded) {
        m_infoLoaded = saved;
    }
    m_artist->setHasChanged(false);
    m_artist->clearImages();
    m_artist->clearExtraFanartData();
    if (saved) {
        emit sigSaved(m_artist);
    }
    return saved;
}

bool ArtistController::infoFromNfoLoaded() const
{
    return m_infoFromNfoLoaded;
}

void ArtistController::setInfoFromNfoLoaded(bool infoFromNfoLoaded)
{
    m_infoFromNfoLoaded = infoFromNfoLoaded;
}

bool ArtistController::infoLoaded() const
{
    return m_infoLoaded;
}

void ArtistController::setInfoLoaded(bool infoLoaded)
{
    m_infoLoaded = infoLoaded;
}

void ArtistController::loadImage(ImageType type, QUrl url)
{
    DownloadManagerElement d;
    d.artist = m_artist;
    d.imageType = type;
    d.url = url;
    emit sigLoadingImages(m_artist, {type});
    m_downloadManager->addDownload(d);
}

void ArtistController::loadImages(ImageType type, QVector<QUrl> urls)
{
    bool started = false;
    for (const QUrl& url : urls) {
        DownloadManagerElement d;
        d.artist = m_artist;
        d.imageType = type;
        d.url = url;
        if (!started) {
            emit sigLoadingImages(m_artist, {type});
            started = true;
        }
        m_downloadManager->addDownload(d);
    }
}

void ArtistController::onAllDownloadsFinished()
{
    m_downloadsInProgress = false;
    m_downloadsSize = 0;
    m_downloadsLeft = 0;
    emit sigLoadDone(m_artist);
}

void ArtistController::onDownloadFinished(DownloadManagerElement elem)
{
    m_downloadsLeft--;
    emit sigDownloadProgress(m_artist, m_downloadsLeft, m_downloadsSize);

    if (!elem.data.isEmpty() && elem.imageType == ImageType::ArtistExtraFanart) {
        helper::resizeBackdrop(elem.data);
        m_artist->addExtraFanart(elem.data);
    } else if (!elem.data.isEmpty()) {
        ImageCache::instance()->invalidateImages(
            Manager::instance()->mediaCenterInterface()->imageFileName(m_artist, elem.imageType));
        if (elem.imageType == ImageType::ArtistFanart) {
            helper::resizeBackdrop(elem.data);
        }
        m_artist->setRawImage(elem.imageType, elem.data);
    }

    emit sigImage(m_artist, elem.imageType, elem.data);
}

bool ArtistController::downloadsInProgress() const
{
    return m_downloadsInProgress;
}

void ArtistController::loadData(QString id, MusicScraperInterface* scraperInterface, QVector<MusicScraperInfos> infos)
{
    m_infosToLoad = infos;
    scraperInterface->loadData(id, m_artist, infos);
}

void ArtistController::scraperLoadDone(MusicScraperInterface* scraper)
{
    emit sigInfoLoadDone(m_artist);

    if (scraper == nullptr) {
        onFanartLoadDone(m_artist, QMap<ImageType, QVector<Poster>>());
        return;
    }

    QVector<ImageType> images;
    if (m_infosToLoad.contains(MusicScraperInfos::Thumb)) {
        images << ImageType::ArtistThumb;
        m_artist->clear({MusicScraperInfos::Thumb});
    }
    if (m_infosToLoad.contains(MusicScraperInfos::Fanart)) {
        images << ImageType::ArtistFanart;
        m_artist->clear({MusicScraperInfos::Fanart});
    }
    if (m_infosToLoad.contains(MusicScraperInfos::ExtraFanarts)) {
        images << ImageType::ArtistExtraFanart;
    }
    if (m_infosToLoad.contains(MusicScraperInfos::Logo)) {
        images << ImageType::ArtistLogo;
        m_artist->clear({MusicScraperInfos::Logo});
    }

    if (!images.isEmpty() && !m_artist->mbId().isEmpty()) {
        ImageProviderInterface* imageProvider = nullptr;
        for (ImageProviderInterface* interface : Manager::instance()->imageProviders()) {
            if (interface->identifier() == "images.fanarttv-music_lib") {
                imageProvider = interface;
                break;
            }
        }
        if (imageProvider == nullptr) {
            onFanartLoadDone(m_artist, QMap<ImageType, QVector<Poster>>());
            return;
        }
        connect(imageProvider,
            &ImageProviderInterface::sigArtistImagesLoaded,
            this,
            &ArtistController::onFanartLoadDone,
            Qt::UniqueConnection);
        imageProvider->artistImages(m_artist, m_artist->mbId(), images);
    } else {
        onFanartLoadDone(m_artist, QMap<ImageType, QVector<Poster>>());
    }
}

void ArtistController::onFanartLoadDone(Artist* artist, QMap<ImageType, QVector<Poster>> posters)
{
    if (artist != m_artist) {
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

        if (it.key() == ImageType::ArtistExtraFanart) {
            for (int i = 0, n = it.value().length(); i < n && i < Settings::instance()->extraFanartsMusicArtists();
                 ++i) {
                DownloadManagerElement d;
                d.imageType = it.key();
                d.url = it.value().at(i).originalUrl;
                d.artist = m_artist;
                downloads.append(d);
            }
        } else {
            DownloadManagerElement d;
            d.imageType = it.key();
            d.url = it.value().at(0).originalUrl;
            d.artist = m_artist;
            downloads.append(d);
        }
        if (!imageTypes.contains(it.key())) {
            imageTypes.append(it.key());
        }
    }

    if (downloads.isEmpty()) {
        emit sigLoadDone(m_artist);
    } else {
        emit sigLoadingImages(m_artist, imageTypes);
        emit sigLoadImagesStarted(m_artist);
    }

    m_downloadsInProgress = !downloads.isEmpty();
    m_downloadsSize = downloads.count();
    m_downloadsLeft = downloads.count();
    m_downloadManager->setDownloads(downloads);
}

void ArtistController::abortDownloads()
{
    m_downloadManager->abortDownloads();
}
