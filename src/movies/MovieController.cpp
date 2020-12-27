#include "MovieController.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QtCore/qmath.h>

#include "data/ImageCache.h"
#include "file/NameFormatter.h"
#include "globals/DownloadManager.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "media_centers/MediaCenterInterface.h"
#include "movies/Movie.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "settings/Settings.h"

MovieController::MovieController(Movie* parent) :
    QObject(parent),
    m_movie{parent},
    m_infoLoaded{false},
    m_infoFromNfoLoaded{false},
    m_downloadManager{new DownloadManager(this)},
    m_forceFanartBackdrop{false},
    m_forceFanartPoster{false},
    m_forceFanartClearArt{false},
    m_forceFanartCdArt{false},
    m_forceFanartLogo{false}
{
    connect(m_downloadManager,
        &DownloadManager::sigDownloadFinished,
        this,
        &MovieController::onDownloadFinished,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
    connect(m_downloadManager,
        &DownloadManager::allMovieDownloadsFinished,
        this,
        &MovieController::onAllDownloadsFinished,
        static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection));
}

bool MovieController::saveData(MediaCenterInterface* mediaCenterInterface)
{
    if (!m_movie->streamDetailsLoaded() && Settings::instance()->autoLoadStreamDetails()) {
        loadStreamDetailsFromFile();
    }
    bool saved = mediaCenterInterface->saveMovie(m_movie);

    qDebug() << "[MovieController] Saved movie? =>" << saved;
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

bool MovieController::loadData(MediaCenterInterface* mediaCenterInterface, bool force, bool reloadFromNfo)
{
    if ((m_infoLoaded || m_movie->hasChanged()) && !force
        && (m_infoFromNfoLoaded || (m_movie->hasChanged() && !m_infoFromNfoLoaded))) {
        return m_infoLoaded;
    }

    NameFormatter nameFormatter(Settings::instance()->excludeWords());
    m_movie->blockSignals(true);

    bool infoLoaded = false;
    if (reloadFromNfo) {
        infoLoaded = mediaCenterInterface->loadMovie(m_movie);
    } else {
        infoLoaded = mediaCenterInterface->loadMovie(m_movie, m_movie->nfoContent());
    }

    if (!infoLoaded) {
        if (!m_movie->files().isEmpty()) {
            QFileInfo fi(m_movie->files().at(0).toString());
            if (QString::compare(fi.fileName(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!pathElements.isEmpty()
                    && QString::compare(pathElements.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0) {
                    pathElements.removeLast();
                }
                if (!pathElements.isEmpty()) {
                    m_movie->setName(nameFormatter.formatName(pathElements.last(), false));
                }
            } else if (QString::compare(fi.fileName(), "index.bdmv", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!pathElements.isEmpty()
                    && QString::compare(pathElements.last(), "BDMV", Qt::CaseInsensitive) == 0) {
                    pathElements.removeLast();
                }
                if (!pathElements.isEmpty()) {
                    m_movie->setName(nameFormatter.formatName(pathElements.last(), false));
                }
            } else if (m_movie->inSeparateFolder()) {
                QStringList splitted = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!splitted.isEmpty()) {
                    m_movie->setName(nameFormatter.formatName(splitted.last(), false));
                } else {
                    if (m_movie->files().size() > 1) {
                        m_movie->setName(nameFormatter.formatName(nameFormatter.removeParts(fi.completeBaseName())));
                    } else {
                        m_movie->setName(nameFormatter.formatName(fi.completeBaseName()));
                    }
                }
            } else {
                if (m_movie->files().size() > 1) {
                    m_movie->setName(nameFormatter.formatName(nameFormatter.removeParts(fi.completeBaseName())));
                } else {
                    m_movie->setName(nameFormatter.formatName(fi.completeBaseName()));
                }
            }
            QRegularExpression rx("tt\\d+");
            QRegularExpressionMatch match = rx.match(fi.completeBaseName());
            if (match.hasMatch()) {
                m_movie->setImdbId(ImdbId(match.captured(0)));
            }
        }
    }
    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    m_movie->setChanged(false);
    m_movie->blockSignals(false);
    return infoLoaded;
}

void MovieController::loadData(QHash<mediaelch::scraper::MovieScraper*, QString> ids,
    mediaelch::scraper::MovieScraper* scraperInterface,
    QSet<MovieScraperInfo> infos)
{
    emit sigLoadStarted(m_movie);
    m_infosToLoad = infos;
    if (scraperInterface->meta().identifier == mediaelch::scraper::TmdbMovie::ID
        && !ids.values().first().startsWith("tt")) {
        m_movie->setTmdbId(TmdbId(ids.values().first()));

    } else if (scraperInterface->meta().identifier == mediaelch::scraper::ImdbMovie::ID
               || (scraperInterface->meta().identifier == mediaelch::scraper::TmdbMovie::ID
                   && ids.values().first().startsWith("tt"))) {
        m_movie->setImdbId(ImdbId(ids.values().first()));
    }
    scraperInterface->loadData(ids, m_movie, infos);
}

void MovieController::loadStreamDetailsFromFile()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;
    m_movie->streamDetails()->loadStreamDetails();
    seconds runtime =
        seconds(m_movie->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::DurationInSeconds).toInt());
    if (runtime > 0s) {
        m_movie->setRuntime(duration_cast<minutes>(runtime));
    }
    m_movie->setStreamDetailsLoaded(true);
    m_movie->setChanged(true);
}

QSet<MovieScraperInfo> MovieController::infosToLoad()
{
    return m_infosToLoad;
}

void MovieController::setInfosToLoad(QSet<MovieScraperInfo> infos)
{
    m_infosToLoad = std::move(infos);
}

void MovieController::scraperLoadDone(mediaelch::scraper::MovieScraper* scraper)
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
    mediaelch::scraper::MovieScraper* sigScraper = scraper;

    scraper = (property("isCustomScraper").toBool())
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::Backdrop)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::Backdrop)
        && (m_forceFanartBackdrop || !scraper->scraperNativelySupports().contains(MovieScraperInfo::Backdrop))) {
        images << ImageType::MovieBackdrop;
        m_movie->clear({MovieScraperInfo::Backdrop});
    }

    scraper = (property("isCustomScraper").toBool())
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::Poster)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::Poster)
        && (m_forceFanartPoster || !scraper->scraperNativelySupports().contains(MovieScraperInfo::Poster))) {
        images << ImageType::MoviePoster;
        m_movie->clear({MovieScraperInfo::Poster});
    }

    scraper = (property("isCustomScraper").toBool())
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::ClearArt)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::ClearArt)
        && (m_forceFanartClearArt || !scraper->scraperNativelySupports().contains(MovieScraperInfo::ClearArt))) {
        images << ImageType::MovieClearArt;
        m_movie->clear({MovieScraperInfo::ClearArt});
    }

    scraper = (property("isCustomScraper").toBool())
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::CdArt)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::CdArt)
        && (m_forceFanartCdArt || !scraper->scraperNativelySupports().contains(MovieScraperInfo::CdArt))) {
        images << ImageType::MovieCdArt;
        m_movie->clear({MovieScraperInfo::CdArt});
    }

    scraper = (property("isCustomScraper").toBool())
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::Logo)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::Logo)
        && (m_forceFanartLogo || !scraper->scraperNativelySupports().contains(MovieScraperInfo::Logo))) {
        images << ImageType::MovieLogo;
        m_movie->clear({MovieScraperInfo::Logo});
    }
    if (infosToLoad().contains(MovieScraperInfo::Banner)) {
        images << ImageType::MovieBanner;
    }
    if (infosToLoad().contains(MovieScraperInfo::Thumb)) {
        images << ImageType::MovieThumb;
    }

    if (!images.isEmpty() && (m_movie->tmdbId().isValid() || m_movie->imdbId().isValid())) {
        connect(Manager::instance()->fanartTv(),
            &mediaelch::scraper::ImageProvider::sigMovieImagesLoaded,
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

    if (infosToLoad().contains(MovieScraperInfo::Poster) && !m_movie->images().posters().isEmpty()) {
        posters.insert(ImageType::MoviePoster, QVector<Poster>() << m_movie->images().posters().at(0));
    }
    if (infosToLoad().contains(MovieScraperInfo::Backdrop) && !m_movie->images().backdrops().isEmpty()) {
        posters.insert(ImageType::MovieBackdrop, QVector<Poster>() << m_movie->images().backdrops().at(0));
    }
    if (infosToLoad().contains(MovieScraperInfo::CdArt) && !m_movie->images().discArts().isEmpty()) {
        posters.insert(ImageType::MovieCdArt, QVector<Poster>() << m_movie->images().discArts().at(0));
    }
    if (infosToLoad().contains(MovieScraperInfo::ClearArt) && !m_movie->images().clearArts().isEmpty()) {
        posters.insert(ImageType::MovieClearArt, QVector<Poster>() << m_movie->images().clearArts().at(0));
    }
    if (infosToLoad().contains(MovieScraperInfo::Logo) && !m_movie->images().logos().isEmpty()) {
        posters.insert(ImageType::MovieLogo, QVector<Poster>() << m_movie->images().logos().at(0));
    }

    QVector<DownloadManagerElement> downloads;
    if (infosToLoad().contains(MovieScraperInfo::Actors) && Settings::instance()->downloadActorImages()) {
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

bool MovieController::infoLoaded() const
{
    return m_infoLoaded;
}

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
        scraperLoadDone(Manager::instance()->scrapers().movieScraper(mediaelch::scraper::TmdbMovie::ID));
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
