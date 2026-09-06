#include "scrapers/tv_show/tmdb/TmdbTvShowScrapeJob.h"

#include "data/Locale.h"
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

            // English title: if scrape locale is already English, title is enough.
            // Otherwise use original_name when original_language is en (see parseInfos),
            // or fetch a separate English infos request if still empty.
            if (config().details.contains(ShowScraperInfo::Title) && config().locale.language() != QLatin1String("en")
                && tvShow().englishTitle().isEmpty()) {
                loadEnglishTitle();
                return;
            }
        } else {
            // only override if there are errors
            setScraperError(error);
        }
        emitFinished();
    });
}

void TmdbTvShowScrapeJob::loadEnglishTitle()
{
    m_api.loadShowInfos(Locale::English, m_id, [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            m_parser.parseEnglishTitle(json);
        }
        // English title is optional; do not fail the whole scrape if this request fails.
        emitFinished();
    });
}

} // namespace scraper
} // namespace mediaelch
