#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"

#include "scrapers/tv_show/tmdb/TmdbTvApi.h"
#include "tv_shows/TvShow.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

TmdbTvShowScrapeJob::TmdbTvShowScrapeJob(TmdbTvApi& api, ShowScrapeJob::Config _config, QObject* parent) :
    ShowScrapeJob(_config, parent), m_api{api}, m_parser(m_api, tvShow(), m_error), m_id{config().identifier.str()}
{
}

void TmdbTvShowScrapeJob::execute()
{
    if (!m_id.isValid()) {
        qWarning() << "[TmdbTv] Provided TMDb id is invalid:" << config().identifier;
        m_error.error = ScraperLoadError::ErrorType::ConfigError;
        m_error.message = tr("Show is missing a TMDb id");
        QTimer::singleShot(0, [this]() { emit sigFinished(this); });
        return;
    }
    loadTvShow();
}

void TmdbTvShowScrapeJob::loadTvShow()
{
    m_api.loadShowInfos(config().locale, m_id, [this](QJsonDocument json) {
        m_parser.parseInfos(json, config().locale);
        emit sigFinished(this);
    });
}

} // namespace scraper
} // namespace mediaelch
