#include "scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h"

#include "tv_shows/TvShow.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

ImdbTvShowScrapeJob::ImdbTvShowScrapeJob(ImdbApi& api, ShowScrapeJob::Config _config, QObject* parent) :
    ShowScrapeJob(_config, parent),
    m_api{api},
    m_parser(tvShow(), m_error),
    m_notLoaded{ShowScraperInfo::Title,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Certification,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Rating,
        ShowScraperInfo::Tags,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::FirstAired,
        ShowScraperInfo::Poster},
    m_id{config().identifier.str()}
{
}

void ImdbTvShowScrapeJob::execute()
{
    if (!m_id.isValid()) {
        qWarning() << "[ImdbTv] Provided IMDb id is invalid:" << config().identifier;
        m_error.error = ScraperError::Type::ConfigError;
        m_error.message = tr("Show is missing an IMDb id");
        QTimer::singleShot(0, [this]() { emit sigFinished(this); });
        return;
    }
    tvShow().setImdbId(m_id);
    // TV Show data is always loaded.
    loadTvShow();
}

void ImdbTvShowScrapeJob::loadTvShow()
{
    const auto setInfosLoaded = [this]() {
        const QSet<ShowScraperInfo> availableScraperInfos = {ShowScraperInfo::Title,
            ShowScraperInfo::Genres,
            ShowScraperInfo::Certification,
            ShowScraperInfo::Overview,
            ShowScraperInfo::Rating,
            ShowScraperInfo::Tags,
            ShowScraperInfo::Runtime,
            ShowScraperInfo::FirstAired,
            ShowScraperInfo::Poster};
        for (const auto loaded : availableScraperInfos) {
            if (shouldLoad(loaded)) {
                setIsLoaded(loaded);
            }
        }
    };

    m_api.loadShowInfos(config().locale, m_id, [this, setInfosLoaded](QString html, ScraperError error) {
        if (!error.hasError()) {
            // We need to add the loaded information but may not want to actually store the show's information.
            m_parser.parseInfos(html);
        } else {
            m_error = error;
        }
        setInfosLoaded();
        checkIfDone();
    });
}


bool ImdbTvShowScrapeJob::shouldLoad(ShowScraperInfo info)
{
    QMutexLocker locker(&m_networkMutex);
    return m_notLoaded.contains(info);
}

void ImdbTvShowScrapeJob::setIsLoaded(ShowScraperInfo info)
{
    QMutexLocker locker(&m_networkMutex);
    if (m_notLoaded.contains(info)) {
        m_notLoaded.remove(info);
    } else {
        qCritical() << "[ImdbTvShowScrapeJob] Loaded detail that should not be loaded?" << static_cast<int>(info);
    }
}

void ImdbTvShowScrapeJob::checkIfDone()
{
    QMutexLocker locker(&m_networkMutex);
    if (m_notLoaded.isEmpty()) {
        locker.unlock();
        emit sigFinished(this);
    }
}

} // namespace scraper
} // namespace mediaelch
