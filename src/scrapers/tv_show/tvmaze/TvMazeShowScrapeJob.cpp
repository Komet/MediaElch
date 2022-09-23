#include "TvMazeShowScrapeJob.h"

#include "data/tv_show/TvShow.h"
#include "log/Log.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "scrapers/tv_show/tvmaze/TvMazeApi.h"
#include "scrapers/tv_show/tvmaze/TvMazeShowScrapeJob.h"

#include <QObject>
#include <utility>

namespace mediaelch {
namespace scraper {


TvMazeShowScrapeJob::TvMazeShowScrapeJob(TvMazeApi& api, Config _config, QObject* parent) :
    ShowScrapeJob(std::move(_config), parent), m_api{api}, m_parser{tvShow()}
{
}

void TvMazeShowScrapeJob::doStart()
{
    TvMazeId id{config().identifier.str()};

    if (!id.isValid()) {
        qCWarning(generic) << "[TvMaze] Provided TvMaze ID is invalid:" << config().identifier;
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("TV show is missing a TVmaze ID");
        setScraperError(error);
        emitFinished();
        return;
    }

    m_api.loadShowInfos(id, [this](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            m_parser.parseInfos(json);
        } else {
            // only override if there are errors
            setScraperError(error);
        }
        emitFinished();
    });
}

} // namespace scraper
} // namespace mediaelch
