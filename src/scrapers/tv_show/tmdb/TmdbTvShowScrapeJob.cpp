#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"

#include "data/tv_show/TvShow.h"
#include "log/Log.h"
#include "scrapers/tmdb/TmdbApi.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

TmdbTvShowScrapeJob::TmdbTvShowScrapeJob(TmdbApi& api, ShowScrapeJob::Config _config, QObject* parent) :
    ShowScrapeJob(_config, parent), m_api{api}, m_parser(m_api, tvShow()), m_id{config().identifier.str()}
{
}

void TmdbTvShowScrapeJob::doStart()
{
    if (!m_id.isValid()) {
        qCWarning(generic) << "[TmdbTv] Provided TMDB id is invalid:" << config().identifier;
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("Show is missing a TMDB id");
        setScraperError(error);
        QTimer::singleShot(0, this, [this]() { emitFinished(); });
        return;
    }
    loadTvShow();
}

void TmdbTvShowScrapeJob::loadTvShow()
{
    m_api.loadShowInfos(config().locale, m_id, [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            m_parser.parseInfos(json, config().locale);
        } else {
            // only override if there are errors
            setScraperError(error);
        }
        emitFinished();
    });
}

} // namespace scraper
} // namespace mediaelch
