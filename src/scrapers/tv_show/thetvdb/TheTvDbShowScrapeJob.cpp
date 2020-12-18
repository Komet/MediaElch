#include "TheTvDbShowScrapeJob.h"

#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"
#include "scrapers/tv_show/thetvdb/TheTvDbShowScrapeJob.h"
#include "tv_shows/TvShow.h"

#include <QObject>
#include <utility>

namespace mediaelch {
namespace scraper {


TheTvDbShowScrapeJob::TheTvDbShowScrapeJob(TheTvDbApi& api, Config _config, QObject* parent) :
    ShowScrapeJob(std::move(_config), parent),
    m_api{api},
    m_parser(tvShow()),
    m_notLoaded{ShowScraperInfo::Title,
        ShowScraperInfo::Certification,
        ShowScraperInfo::FirstAired,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Network,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Rating,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::Status,
        ShowScraperInfo::Actors,
        ShowScraperInfo::Fanart,
        ShowScraperInfo::Poster,
        ShowScraperInfo::SeasonPoster,
        ShowScraperInfo::SeasonBanner,
        ShowScraperInfo::Banner},
    m_id{config().identifier.str()}
{
    m_notLoaded.intersect(config().details);
}

void TheTvDbShowScrapeJob::execute()
{
    if (!m_id.isValid()) {
        qWarning() << "[TheTvDb] Provided TheTvDb id is invalid:" << config().identifier;
        m_error.error = ScraperError::Type::ConfigError;
        m_error.message = tr("Show is missing a TheTvDb id");
        QTimer::singleShot(0, [this]() { emit sigFinished(this); });
        return;
    }

    // TV Show data is always loaded.
    loadTvShow();

    if (shouldLoad(ShowScraperInfo::Actors)) {
        loadActors();
    }
    if (shouldLoad(ShowScraperInfo::Fanart)) {
        loadImages(ShowScraperInfo::Fanart);
    }
    if (shouldLoad(ShowScraperInfo::Poster)) {
        loadImages(ShowScraperInfo::Poster);
    }
    if (shouldLoad(ShowScraperInfo::SeasonPoster)) {
        loadImages(ShowScraperInfo::SeasonPoster);
    }
    if (shouldLoad(ShowScraperInfo::SeasonBanner)) {
        loadImages(ShowScraperInfo::SeasonBanner);
        // also load normal banners if season banners are requested
        m_notLoaded.insert(ShowScraperInfo::Banner);
    }
    if (shouldLoad(ShowScraperInfo::Banner)) {
        loadImages(ShowScraperInfo::Banner);
    }
}

void TheTvDbShowScrapeJob::loadTvShow()
{
    const auto setInfosLoaded = [this]() {
        const QSet<ShowScraperInfo> availableScraperInfos = {ShowScraperInfo::Certification,
            ShowScraperInfo::FirstAired,
            ShowScraperInfo::Genres,
            ShowScraperInfo::Network,
            ShowScraperInfo::Overview,
            ShowScraperInfo::Rating,
            ShowScraperInfo::Title,
            ShowScraperInfo::Runtime,
            ShowScraperInfo::Status};

        for (const auto loaded : availableScraperInfos) {
            if (shouldLoad(loaded)) {
                setIsLoaded(loaded);
            }
        }
    };

    m_api.loadShowInfos(config().locale, m_id, [this, setInfosLoaded](QJsonDocument json, ScraperError error) {
        // We need to add the loaded information but may not want to actually store the show's information.
        if (!error.hasError()) {
            m_parser.parseInfos(json.object());
        } else {
            // only override if there are errors
            m_error = error;
        }
        setInfosLoaded();
        checkIfDone();
    });
}

void TheTvDbShowScrapeJob::loadActors()
{
    m_api.loadShowActors(config().locale, m_id, [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            m_parser.parseActors(json.object());
        } else {
            // only override if there are errors
            m_error = error;
        }
        setIsLoaded(ShowScraperInfo::Actors);
        checkIfDone();
    });
}

void TheTvDbShowScrapeJob::loadImages(ShowScraperInfo imageType)
{
    m_api.loadImageUrls(config().locale, m_id, imageType, [this, imageType](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            m_parser.parseImages(json.object());
        } else {
            // only override if there are errors
            m_error = error;
        }
        setIsLoaded(imageType);
        checkIfDone();
    });
}

bool TheTvDbShowScrapeJob::shouldLoad(ShowScraperInfo info)
{
    QMutexLocker locker(&m_networkMutex);
    return m_notLoaded.contains(info);
}

void TheTvDbShowScrapeJob::setIsLoaded(ShowScraperInfo info)
{
    QMutexLocker locker(&m_networkMutex);
    if (m_notLoaded.contains(info)) {
        m_notLoaded.remove(info);
    } else {
        qCritical() << "[TheTvDbShowScrapeJob] Loaded detail that should not be loaded?" << static_cast<int>(info);
    }
}

void TheTvDbShowScrapeJob::checkIfDone()
{
    QMutexLocker locker(&m_networkMutex);
    if (m_notLoaded.isEmpty()) {
        locker.unlock();
        emit sigFinished(this);
    }
}

} // namespace scraper
} // namespace mediaelch
