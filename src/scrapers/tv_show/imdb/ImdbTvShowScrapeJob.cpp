#include "scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h"

#include "data/tv_show/TvShow.h"
#include "log/Log.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

ImdbTvShowScrapeJob::ImdbTvShowScrapeJob(ImdbApi& api, ShowScrapeJob::Config _config, QObject* parent) :
    ShowScrapeJob(_config, parent),
    m_api{api},
    m_parser(tvShow()),
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

void ImdbTvShowScrapeJob::doStart()
{
    if (!m_id.isValid()) {
        qCWarning(generic) << "[ImdbTv] Provided IMDb id is invalid:" << config().identifier;
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("Show is missing an IMDb id");
        setScraperError(error);
        QTimer::singleShot(0, this, [this]() { emitFinished(); });
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

    const auto callback = [this, setInfosLoaded](QString html, ScraperError error) {
        if (!error.hasError()) {
            // We need to add the loaded information but may not want to actually store the show's information.
            error = m_parser.parseInfos(html);
        }
        if (error.hasError()) {
            setScraperError(error);
        }
        setInfosLoaded();
        checkIfDone();
    };

    m_api.loadTitle(config().locale, m_id, ImdbApi::PageKind::Main, callback);
}


bool ImdbTvShowScrapeJob::shouldLoad(ShowScraperInfo info)
{
    return m_notLoaded.contains(info);
}

void ImdbTvShowScrapeJob::setIsLoaded(ShowScraperInfo info)
{
    if (m_notLoaded.contains(info)) {
        m_notLoaded.remove(info);
    } else {
        qCCritical(generic) << "[ImdbTvShowScrapeJob] Loaded detail that should not be loaded?"
                            << static_cast<int>(info);
    }
}

void ImdbTvShowScrapeJob::checkIfDone()
{
    if (m_notLoaded.isEmpty()) {
        emitFinished();
    }
}

} // namespace scraper
} // namespace mediaelch
