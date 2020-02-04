#include "MovieController.h"

#include <QDir>
#include <QFileInfo>
#include <QtCore/qmath.h>

#include "data/ImageCache.h"
#include "globals/DownloadManager.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/NameFormatter.h"
#include "media_centers/MediaCenterInterface.h"
#include "movies/Movie.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/MovieScraperInterface.h"
#include "scrapers/movie/TMDb.h"
#include "settings/Settings.h"

MovieController::MovieController(Movie* parent) :
    QObject(parent),
    m_movie{parent},
    m_infoLoaded{false},
    m_infoFromNfoLoaded{false},
    m_downloadManager{new DownloadManager(this)},
    m_downloadsInProgress{false},
    m_downloadsSize{0},
    m_forceFanartBackdrop{false},
    m_forceFanartPoster{false},
    m_forceFanartClearArt{false},
    m_forceFanartCdArt{false},
    m_forceFanartLogo{false}
{
    connect(m_downloadManager, &DownloadManager::sigDownloadFinished, this, &MovieController::onDownloadFinished);
    connect(m_downloadManager,
        SIGNAL(allDownloadsFinished(Movie*)),
        this,
        SLOT(onAllDownloadsFinished()),
        Qt::UniqueConnection);
}

/**
 * @brief Saves the movies infos with the given MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use for saving
 * @return Saving was successful or not
 */
bool MovieController::saveData(MediaCenterInterface* mediaCenterInterface)
{
    qDebug() << "Entered";

    if (!m_movie->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails()) {
        loadStreamDetailsFromFile();
    }
    bool saved = mediaCenterInterface->saveMovie(m_movie);
    qDebug() << "Saved" << saved;
    if (!m_infoLoaded) {
        m_infoLoaded = saved;
    }
    m_movie->setChanged(false);
    m_movie->clearImages();
    m_movie->images().clearExtraFanartData();
    m_movie->setSyncNeeded(true);
    for (Subtitle* subtitle : m_movie->subtitles()) {
        subtitle->setChanged(false);
    }
    return saved;
}

/**
 * @brief Loads the movies infos with the given MediaCenterInterface
 * @param mediaCenterInterface MediaCenterInterface to use for loading
 * @param force Force the loading. If set to false and infos were already loeaded this function just returns
 * @return Loading was successful or not
 */
bool MovieController::loadData(MediaCenterInterface* mediaCenterInterface, bool force, bool reloadFromNfo)
{
    if ((m_infoLoaded || m_movie->hasChanged()) && !force
        && (m_infoFromNfoLoaded || (m_movie->hasChanged() && !m_infoFromNfoLoaded))) {
        return m_infoLoaded;
    }

    m_movie->blockSignals(true);
    NameFormatter* nameFormat = NameFormatter::instance();

    bool infoLoaded;
    if (reloadFromNfo) {
        infoLoaded = mediaCenterInterface->loadMovie(m_movie);
    } else {
        infoLoaded = mediaCenterInterface->loadMovie(m_movie, m_movie->nfoContent());
    }

    if (!infoLoaded) {
        if (!m_movie->files().empty()) {
            QFileInfo fi(m_movie->files().at(0));
            if (QString::compare(fi.fileName(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!pathElements.empty()
                    && QString::compare(pathElements.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0) {
                    pathElements.removeLast();
                }
                if (!pathElements.empty()) {
                    m_movie->setName(nameFormat->formatName(pathElements.last(), false));
                }
            } else if (QString::compare(fi.fileName(), "index.bdmv", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!pathElements.empty() && QString::compare(pathElements.last(), "BDMV", Qt::CaseInsensitive) == 0) {
                    pathElements.removeLast();
                }
                if (!pathElements.empty()) {
                    m_movie->setName(nameFormat->formatName(pathElements.last(), false));
                }
            } else if (m_movie->inSeparateFolder()) {
                QStringList splitted = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!splitted.isEmpty()) {
                    m_movie->setName(nameFormat->formatName(splitted.last(), false));
                } else {
                    if (m_movie->files().size() > 1) {
                        m_movie->setName(nameFormat->formatName(nameFormat->formatParts(fi.completeBaseName())));
                    } else {
                        m_movie->setName(nameFormat->formatName(fi.completeBaseName()));
                    }
                }
            } else {
                if (m_movie->files().size() > 1) {
                    m_movie->setName(nameFormat->formatName(nameFormat->formatParts(fi.completeBaseName())));
                } else {
                    m_movie->setName(nameFormat->formatName(fi.completeBaseName()));
                }
            }
            QRegExp rx("(tt[0-9]+)");
            if (rx.indexIn(fi.completeBaseName()) != -1) {
                m_movie->setId(ImdbId(rx.cap(1)));
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
 * @param ids Id of the movie within the given ScraperInterface
 * @param scraperInterface ScraperInterface to use for loading
 * @param infos List of infos to load
 */
void MovieController::loadData(QMap<MovieScraperInterface*, QString> ids,
    MovieScraperInterface* scraperInterface,
    QVector<MovieScraperInfos> infos)
{
    m_infosToLoad = infos;
    if (scraperInterface->identifier() == TMDb::scraperIdentifier && !ids.values().first().startsWith("tt")) {
        m_movie->setTmdbId(TmdbId(ids.values().first()));

    } else if (scraperInterface->identifier() == IMDB::scraperIdentifier
               || (scraperInterface->identifier() == TMDb::scraperIdentifier
                   && ids.values().first().startsWith("tt"))) {
        m_movie->setId(ImdbId(ids.values().first()));
    }
    scraperInterface->loadData(ids, m_movie, infos);
}

/**
 * @brief Tries to load streamdetails from the file
 */
void MovieController::loadStreamDetailsFromFile()
{
    using namespace std::chrono;
    m_movie->streamDetails()->loadStreamDetails();
    seconds runtime =
        seconds(m_movie->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::DurationInSeconds).toInt());
    m_movie->setRuntime(duration_cast<minutes>(runtime));
    m_movie->setStreamDetailsLoaded(true);
    m_movie->setChanged(true);
}

QVector<MovieScraperInfos> MovieController::infosToLoad()
{
    return m_infosToLoad;
}

void MovieController::setInfosToLoad(QVector<MovieScraperInfos> infos)
{
    m_infosToLoad = std::move(infos);
}

/**
 * @brief Called when a ScraperInterface has finished loading
 *        Emits the loaded signal
 */
void MovieController::scraperLoadDone(MovieScraperInterface* scraper)
{
    m_customScraperMutex.lock();
    if (!property("customMovieScraperLoads").isNull() && property("customMovieScraperLoads").toInt() > 1) {
        setProperty("customMovieScraperLoads", property("customMovieScraperLoads").toInt() - 1);
        m_customScraperMutex.unlock();
        return;
    }
    m_customScraperMutex.unlock();


    setProperty("customMovieScraperLoads", QVariant());

    emit sigInfoLoadDone(m_movie);

    if (scraper == nullptr) {
        onFanartLoadDone(m_movie, QMap<ImageType, QVector<Poster>>());
        return;
    }

    QVector<ImageType> images;
    MovieScraperInterface* sigScraper = scraper;

    scraper = (property("isCustomScraper").toBool())
                  ? CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfos::Backdrop)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfos::Backdrop)
        && (m_forceFanartBackdrop || !scraper->scraperNativelySupports().contains(MovieScraperInfos::Backdrop))) {
        images << ImageType::MovieBackdrop;
        m_movie->clear({MovieScraperInfos::Backdrop});
    }

    scraper = (property("isCustomScraper").toBool())
                  ? CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfos::Poster)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfos::Poster)
        && (m_forceFanartPoster || !scraper->scraperNativelySupports().contains(MovieScraperInfos::Poster))) {
        images << ImageType::MoviePoster;
        m_movie->clear({MovieScraperInfos::Poster});
    }

    scraper = (property("isCustomScraper").toBool())
                  ? CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfos::ClearArt)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfos::ClearArt)
        && (m_forceFanartClearArt || !scraper->scraperNativelySupports().contains(MovieScraperInfos::ClearArt))) {
        images << ImageType::MovieClearArt;
        m_movie->clear({MovieScraperInfos::ClearArt});
    }

    scraper = (property("isCustomScraper").toBool())
                  ? CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfos::CdArt)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfos::CdArt)
        && (m_forceFanartCdArt || !scraper->scraperNativelySupports().contains(MovieScraperInfos::CdArt))) {
        images << ImageType::MovieCdArt;
        m_movie->clear({MovieScraperInfos::CdArt});
    }

    scraper = (property("isCustomScraper").toBool())
                  ? CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfos::Logo)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfos::Logo)
        && (m_forceFanartLogo || !scraper->scraperNativelySupports().contains(MovieScraperInfos::Logo))) {
        images << ImageType::MovieLogo;
        m_movie->clear({MovieScraperInfos::Logo});
    }
    if (infosToLoad().contains(MovieScraperInfos::Banner)) {
        images << ImageType::MovieBanner;
    }
    if (infosToLoad().contains(MovieScraperInfos::Thumb)) {
        images << ImageType::MovieThumb;
    }

    if (!images.isEmpty() && (m_movie->tmdbId().isValid() || m_movie->imdbId().isValid())) {
        connect(Manager::instance()->fanartTv(),
            &ImageProviderInterface::sigMovieImagesLoaded,
            this,
            &MovieController::onFanartLoadDone,
            Qt::UniqueConnection);
        Manager::instance()->fanartTv()->movieImages(
            m_movie, (m_movie->tmdbId().isValid()) ? m_movie->tmdbId() : TmdbId(m_movie->imdbId().toString()), images);
    } else {
        onFanartLoadDone(m_movie, QMap<ImageType, QVector<Poster>>());
    }
}

void MovieController::onFanartLoadDone(Movie* movie, QMap<ImageType, QVector<Poster>> posters)
{
    if (movie != m_movie) {
        return;
    }

    m_forceFanartPoster = false;
    m_forceFanartBackdrop = false;
    m_forceFanartLogo = false;
    m_forceFanartCdArt = false;
    m_forceFanartClearArt = false;

    if (infosToLoad().contains(MovieScraperInfos::Poster) && !m_movie->images().posters().isEmpty()) {
        posters.insert(ImageType::MoviePoster, QVector<Poster>() << m_movie->images().posters().at(0));
    }
    if (infosToLoad().contains(MovieScraperInfos::Backdrop) && !m_movie->images().backdrops().isEmpty()) {
        posters.insert(ImageType::MovieBackdrop, QVector<Poster>() << m_movie->images().backdrops().at(0));
    }
    if (infosToLoad().contains(MovieScraperInfos::CdArt) && !m_movie->images().discArts().isEmpty()) {
        posters.insert(ImageType::MovieCdArt, QVector<Poster>() << m_movie->images().discArts().at(0));
    }
    if (infosToLoad().contains(MovieScraperInfos::ClearArt) && !m_movie->images().clearArts().isEmpty()) {
        posters.insert(ImageType::MovieClearArt, QVector<Poster>() << m_movie->images().clearArts().at(0));
    }
    if (infosToLoad().contains(MovieScraperInfos::Logo) && !m_movie->images().logos().isEmpty()) {
        posters.insert(ImageType::MovieLogo, QVector<Poster>() << m_movie->images().logos().at(0));
    }

    QVector<DownloadManagerElement> downloads;
    if (infosToLoad().contains(MovieScraperInfos::Actors) && Settings::instance()->downloadActorImages()) {
        for (Actor* actor : m_movie->actors()) {
            if (actor->thumb.isEmpty()) {
                continue;
            }
            DownloadManagerElement d;
            d.imageType = ImageType::Actor;
            d.url = QUrl(actor->thumb);
            d.actor = actor;
            d.movie = movie;
            downloads.append(d);
        }
    }

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
        d.movie = m_movie;
        downloads.append(d);
        if (!imageTypes.contains(it.key())) {
            imageTypes.append(it.key());
        }
    }

    setProperty("isCustomScraper", false);

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

    if (!elem.data.isEmpty() && elem.imageType == ImageType::Actor) {
        elem.actor->image = elem.data;
    } else if (!elem.data.isEmpty() && elem.imageType == ImageType::MovieExtraFanart) {
        helper::resizeBackdrop(elem.data);
        m_movie->images().addExtraFanart(elem.data);
    } else if (!elem.data.isEmpty()) {
        ImageCache::instance()->invalidateImages(
            Manager::instance()->mediaCenterInterface()->imageFileName(m_movie, elem.imageType));
        if (elem.imageType == ImageType::MovieBackdrop) {
            helper::resizeBackdrop(elem.data);
        }
        m_movie->images().setImage(elem.imageType, elem.data);
    }

    if (elem.imageType != ImageType::Actor) {
        emit sigImage(m_movie, elem.imageType, elem.data);
    }
}

void MovieController::loadImage(ImageType type, QUrl url)
{
    DownloadManagerElement d;
    d.movie = m_movie;
    d.imageType = type;
    d.url = std::move(url);
    emit sigLoadingImages(m_movie, {type});
    m_downloadManager->addDownload(d);
}

void MovieController::loadImages(ImageType type, QVector<QUrl> urls)
{
    for (const auto& url : urls) {
        DownloadManagerElement d;
        d.movie = m_movie;
        d.imageType = type;
        d.url = url;
        emit sigLoadingImages(m_movie, {type});
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

void MovieController::setLoadsLeft(QVector<ScraperData> loadsLeft)
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
        scraperLoadDone(Manager::instance()->scraper(TMDb::scraperIdentifier));
    }
    m_loadMutex.unlock();
}

void MovieController::setForceFanartBackdrop(const bool& force)
{
    m_forceFanartBackdrop = force;
}

void MovieController::setForceFanartPoster(const bool& force)
{
    m_forceFanartPoster = force;
}

void MovieController::setForceFanartCdArt(const bool& force)
{
    m_forceFanartCdArt = force;
}

void MovieController::setForceFanartClearArt(const bool& force)
{
    m_forceFanartClearArt = force;
}

void MovieController::setForceFanartLogo(const bool& force)
{
    m_forceFanartLogo = force;
}
