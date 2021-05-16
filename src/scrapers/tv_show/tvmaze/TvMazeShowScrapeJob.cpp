#include "TvMazeShowScrapeJob.h"

#include "log/Log.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "scrapers/tv_show/tvmaze/TvMazeApi.h"
#include "scrapers/tv_show/tvmaze/TvMazeShowScrapeJob.h"
#include "tv_shows/TvShow.h"

#include <QObject>
#include <utility>

namespace mediaelch {
namespace scraper {


TvMazeShowScrapeJob::TvMazeShowScrapeJob(TvMazeApi& api, Config _config, QObject* parent) :
    ShowScrapeJob(std::move(_config), parent), m_api{api}, m_parser{tvShow()}
{
}

void TvMazeShowScrapeJob::start()
{
    TvMazeId id{config().identifier.str()};

    if (!id.isValid()) {
        qCWarning(generic) << "[TvMaze] Provided TvMaze ID is invalid:" << config().identifier;
        m_error.error = ScraperError::Type::ConfigError;
        m_error.message = tr("TV show is missing a TVmaze ID");
        emit sigFinished(this);
        return;
    }

    m_api.loadShowInfos(id, [this](QJsonDocument json, ScraperError error) {
        if (!m_error.hasError()) {
            m_parser.parseInfos(json);
        } else {
            m_error = error;
        }
        emit sigFinished(this);
    });
}

} // namespace scraper
} // namespace mediaelch
