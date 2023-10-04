#include "MovieController.h"

#include "data/movie/Movie.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "media/ImageCache.h"
#include "media/ImageUtils.h"
#include "media/NameFormatter.h"
#include "media_center/MediaCenterInterface.h"
#include "network/DownloadManager.h"
#include "scrapers/movie/MovieMerger.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/custom/CustomMovieScrapeJob.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/movie/tmdb/TmdbMovie.h"
#include "settings/Settings.h"
// TODO: Remove UI dependency
#include "ui/notifications/NotificationBox.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QtCore/qmath.h>
#include <chrono>

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
        const bool success = loadStreamDetailsFromFile();
        if (!success) {
            // TODO: Tell the user that it failed
            qCDebug(generic) << "[MovieController] Could not load stream details for movie with ID="
                             << m_movie->movieId();
        }
    }
    bool saved = mediaCenterInterface->saveMovie(m_movie);

    qCDebug(generic) << "[MovieController] Saved movie? =>" << saved;
    if (!m_infoLoaded) {
        m_infoLoaded = saved;
    }
    m_movie->setChanged(false);
    m_movie->clearImages();
    m_movie->images().clearExtraFanartData();
    m_movie->setSyncNeeded(true);

    const auto subtitles = m_movie->subtitles();
    for (Subtitle* subtitle : subtitles) {
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

    NameFormatter::setExcludeWords(Settings::instance()->excludeWords());

    const QSignalBlocker blocker(m_movie);

    bool infoLoaded = false;
    if (reloadFromNfo) {
        infoLoaded = mediaCenterInterface->loadMovie(m_movie);
    } else {
        infoLoaded = mediaCenterInterface->loadMovie(m_movie, m_movie->nfoContent());
    }

    // Movies should always have a name, even if a valid NFO file does not have a name tag.
    if (!infoLoaded || m_movie->name().isEmpty()) {
        if (!m_movie->files().isEmpty()) {
            QFileInfo fi(m_movie->files().at(0).toString());
            if (QString::compare(fi.fileName(), "VIDEO_TS.IFO", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!pathElements.isEmpty()
                    && QString::compare(pathElements.last(), "VIDEO_TS", Qt::CaseInsensitive) == 0) {
                    pathElements.removeLast();
                }
                if (!pathElements.isEmpty()) {
                    m_movie->setName(NameFormatter::formatName(pathElements.last(), false));
                }
            } else if (QString::compare(fi.fileName(), "index.bdmv", Qt::CaseInsensitive) == 0) {
                QStringList pathElements = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!pathElements.isEmpty()
                    && QString::compare(pathElements.last(), "BDMV", Qt::CaseInsensitive) == 0) {
                    pathElements.removeLast();
                }
                if (!pathElements.isEmpty()) {
                    m_movie->setName(NameFormatter::formatName(pathElements.last(), false));
                }
            } else if (m_movie->inSeparateFolder()) {
                QStringList splitted = QDir::toNativeSeparators(fi.path()).split(QDir::separator());
                if (!splitted.isEmpty()) {
                    m_movie->setName(NameFormatter::formatName(splitted.last(), false));
                } else {
                    if (m_movie->files().size() > 1) {
                        m_movie->setName(NameFormatter::formatName(NameFormatter::removeParts(fi.completeBaseName())));
                    } else {
                        m_movie->setName(NameFormatter::formatName(fi.completeBaseName()));
                    }
                }
            } else {
                if (m_movie->files().size() > 1) {
                    m_movie->setName(NameFormatter::formatName(NameFormatter::removeParts(fi.completeBaseName())));
                } else {
                    m_movie->setName(NameFormatter::formatName(fi.completeBaseName()));
                }
            }

            if (!m_movie->imdbId().isValid()) {
                QRegularExpression rx("tt\\d+");
                QRegularExpressionMatch match = rx.match(fi.completeBaseName());
                if (match.hasMatch()) {
                    m_movie->setImdbId(ImdbId(match.captured(0)));
                }
            }
        }
    }
    m_infoLoaded = infoLoaded;
    m_infoFromNfoLoaded = infoLoaded && reloadFromNfo;
    m_movie->setChanged(false);
    return infoLoaded;
}

void MovieController::loadData(QHash<mediaelch::scraper::MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    const mediaelch::Locale& locale,
    const QSet<MovieScraperInfo>& details)
{
    using namespace mediaelch::scraper;
    if (ids.isEmpty()) {
        qCWarning(generic) << "[MovieController] Tried to start scraping without providing any IDs";
        return;
    }

    emit sigLoadStarted(m_movie);

    m_infosToLoad = details;

    const MovieIdentifier id = ids.constBegin().value();

    MovieScraper* scraper = nullptr;
    MovieScrapeJob* scrapeJob = nullptr;

    if (ids.size() > 1) {
        // Requires custom movie scraper
        // TODO: Maybe use some custom loadMovie() function? This seems hacky.
        //       There could be only a single scraper being used.
        CustomMovieScraper::instance()->setScraperMovieIds(std::move(ids));
        scraper = CustomMovieScraper::instance();

        // Currently hacky, see this issue for details:
        // https://github.com/Komet/MediaElch/issues/1598
        auto detailScraperMap = Settings::instance()->customMovieScraper();
        if (details.contains(MovieScraperInfo::Backdrop)
            && detailScraperMap.value(MovieScraperInfo::Backdrop) == "images.fanarttv") {
            setForceFanartBackdrop(true);
        }
        if (details.contains(MovieScraperInfo::Poster)
            && detailScraperMap.value(MovieScraperInfo::Poster) == "images.fanarttv") {
            setForceFanartPoster(true);
        }
        if (details.contains(MovieScraperInfo::ClearArt)
            && detailScraperMap.value(MovieScraperInfo::ClearArt) == "images.fanarttv") {
            setForceFanartClearArt(true);
        }
        if (details.contains(MovieScraperInfo::CdArt)
            && detailScraperMap.value(MovieScraperInfo::CdArt) == "images.fanarttv") {
            setForceFanartCdArt(true);
        }
        if (details.contains(MovieScraperInfo::Logo)
            && detailScraperMap.value(MovieScraperInfo::Logo) == "images.fanarttv") {
            setForceFanartLogo(true);
        }


        // Only needed for details list.
        MovieScrapeJob::Config config;
        config.details = details;
        scrapeJob = scraper->loadMovie(config);

    } else {
        MovieScrapeJob::Config config;
        config.details = details;
        config.locale = locale;
        config.identifier = id;

        scraper = ids.constBegin().key();
        const auto scraperId = scraper->meta().identifier;
        const bool isImdbId = ImdbId::isValidFormat(id.str());

        if (scraperId == TmdbMovie::ID && !isImdbId) {
            m_movie->setTmdbId(TmdbId(id.str()));

        } else if (scraperId == ImdbMovie::ID || (scraperId == TmdbMovie::ID && isImdbId)) {
            m_movie->setImdbId(ImdbId(id.str()));
        }

        scrapeJob = scraper->loadMovie(config);
    }

    connect(scrapeJob, &MovieScrapeJob::loadFinished, this, [this, scraper](MovieScrapeJob* job) { //
        job->deleteLater();
        copyDetailsToMovie(*m_movie,
            job->movie(),
            job->config().details,
            Settings::instance()->usePlotForOutline(),
            Settings::instance()->ignoreDuplicateOriginalTitle());
        scraperLoadDone(scraper, job);
    });
    scrapeJob->start();
}

bool MovieController::loadStreamDetailsFromFile()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;
    bool success = m_movie->streamDetails()->loadStreamDetails();
    if (!success) {
        return false;
    }
    seconds runtime =
        seconds(m_movie->streamDetails()->videoDetails().value(StreamDetails::VideoDetails::DurationInSeconds).toInt());
    if (runtime > 0s) {
        m_movie->setRuntime(duration_cast<minutes>(runtime));
    }
    m_movie->setChanged(true);
    return true;
}

QSet<MovieScraperInfo> MovieController::infosToLoad()
{
    return m_infosToLoad;
}


void MovieController::scraperLoadDone(mediaelch::scraper::MovieScraper* scraper,
    mediaelch::scraper::MovieScrapeJob* job)
{
    using namespace std::chrono_literals;

    if (job->hasError() && !job->scraperError().is404()) {
        // TODO: 404 not necessary but avoids false positives at the moment.
        // TODO: Remove UI dependency
        NotificationBox::instance()->showError(job->errorString(), 6s);
    }

    m_movie->setChanged(true);
    emit sigInfoLoadDone(m_movie);

    if (scraper == nullptr) {
        onFanartLoadDone(m_movie, QMap<ImageType, QVector<Poster>>());
        return;
    }

    // TODO: Fix hacky coding below; it has repetition; is not part of scraper and has other issues.
    const bool isCustomMovieScraper = scraper->meta().identifier == mediaelch::scraper::CustomMovieScraper::ID;

    QSet<ImageType> images;
    mediaelch::scraper::MovieScraper* sigScraper = scraper;

    scraper = isCustomMovieScraper
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::Backdrop)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::Backdrop)
        && (m_forceFanartBackdrop
            || (scraper != nullptr && !scraper->scraperNativelySupports().contains(MovieScraperInfo::Backdrop)))) {
        images << ImageType::MovieBackdrop;
        m_movie->clear({MovieScraperInfo::Backdrop});
    }

    scraper = isCustomMovieScraper
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::Poster)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::Poster)
        && (m_forceFanartPoster
            || (scraper != nullptr && !scraper->scraperNativelySupports().contains(MovieScraperInfo::Poster)))) {
        images << ImageType::MoviePoster;
        m_movie->clear({MovieScraperInfo::Poster});
    }

    scraper = isCustomMovieScraper
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::ClearArt)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::ClearArt)
        && (m_forceFanartClearArt
            || (scraper != nullptr && !scraper->scraperNativelySupports().contains(MovieScraperInfo::ClearArt)))) {
        images << ImageType::MovieClearArt;
        m_movie->clear({MovieScraperInfo::ClearArt});
    }

    scraper = isCustomMovieScraper
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::CdArt)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::CdArt)
        && (m_forceFanartCdArt
            || (scraper != nullptr && !scraper->scraperNativelySupports().contains(MovieScraperInfo::CdArt)))) {
        images << ImageType::MovieCdArt;
        m_movie->clear({MovieScraperInfo::CdArt});
    }

    scraper = isCustomMovieScraper
                  ? mediaelch::scraper::CustomMovieScraper::instance()->scraperForInfo(MovieScraperInfo::Logo)
                  : sigScraper;
    if (infosToLoad().contains(MovieScraperInfo::Logo)
        && (m_forceFanartLogo
            || (scraper != nullptr && !scraper->scraperNativelySupports().contains(MovieScraperInfo::Logo)))) {
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

    QSet<ImageType> imageTypes;
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
            imageTypes.insert(it.key());
        }
    }

    if (downloadsInProgress()) {
        // If downloads are already in progress, it may be that some downloads haven't finished.
        // We must take care not to emit sigLoadDone() twice.
        qCCritical(generic)
            << "[MovieController] Download is already in progress! Won't start other downloads for movie:"
            << m_movie->name();
        return;
    }

    if (downloads.isEmpty()) {
        onAllDownloadsFinished();
        return;
    }

    emit sigLoadingImages(m_movie, imageTypes);
    emit sigLoadImagesStarted(m_movie);

    m_downloadsSize = qsizetype_to_int(downloads.count());
    m_downloadManager->setDownloads(downloads);
}

void MovieController::onAllDownloadsFinished()
{
    m_downloadsSize = 0;
    emit sigLoadDone(m_movie);
}

void MovieController::onDownloadFinished(DownloadManagerElement elem)
{
    emit sigDownloadProgress(m_movie, m_downloadManager->downloadQueueSize(), m_downloadsSize);

    if (!elem.data.isEmpty()) {
        if (elem.imageType == ImageType::Actor) {
            elem.actor->image = elem.data;
        } else if (elem.imageType == ImageType::MovieExtraFanart) {
            mediaelch::resizeBackdrop(elem.data);
            m_movie->images().addExtraFanart(elem.data);
        } else {
            ImageCache::instance()->invalidateImages(mediaelch::FilePath(
                Manager::instance()->mediaCenterInterface()->imageFileName(m_movie, elem.imageType)));
            if (elem.imageType == ImageType::MovieBackdrop) {
                mediaelch::resizeBackdrop(elem.data);
            }
            m_movie->images().setImage(elem.imageType, elem.data);
        }

        if (elem.imageType != ImageType::Actor) {
            emit sigImage(m_movie, elem.imageType, elem.data);
        }
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
    return m_downloadManager->isDownloading();
}

void MovieController::abortDownloads()
{
    m_downloadManager->abortDownloads();
    emit sigLoadDone(m_movie);
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
