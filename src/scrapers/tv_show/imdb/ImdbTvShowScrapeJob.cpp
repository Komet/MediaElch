#include "scrapers/tv_show/imdb/ImdbTvShowScrapeJob.h"

#include "data/tv_show/TvShow.h"
#include "log/Log.h"
#include "scrapers/imdb/ImdbJsonParser.h"
#include "utils/Containers.h"

#include <QTimer>

namespace mediaelch {
namespace scraper {

ImdbTvShowScrapeJob::ImdbTvShowScrapeJob(ImdbApi& api, ShowScrapeJob::Config _config, QObject* parent) :
    ShowScrapeJob(_config, parent), m_api{api}, m_id{config().identifier.str()}
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

    m_api.loadTitleViaGraphQL(m_id, [this](QString data, ScraperError error) {
        if (error.hasError()) {
            setScraperError(error);
            emitFinished();
            return;
        }
        parseAndAssignInfos(data);
        emitFinished();
    });
}

void ImdbTvShowScrapeJob::parseAndAssignInfos(const QString& json)
{
    ImdbData data = ImdbJsonParser::parseFromGraphQL(json, config().locale);

    if (data.imdbId.isValid()) {
        tvShow().setImdbId(data.imdbId);
    }

    // Title: use localized title if available
    if (data.localizedTitle.hasValue()) {
        tvShow().setTitle(data.localizedTitle.value);
        if (data.originalTitle.hasValue()) {
            tvShow().setOriginalTitle(data.originalTitle.value);
        } else if (data.title.hasValue()) {
            tvShow().setOriginalTitle(data.title.value);
        }
    } else if (data.title.hasValue()) {
        tvShow().setTitle(data.title.value);
        if (data.originalTitle.hasValue()) {
            tvShow().setOriginalTitle(data.originalTitle.value);
        }
    }

    if (data.overview.hasValue()) {
        tvShow().setOverview(data.overview.value);
    }
    if (data.certification.hasValue()) {
        tvShow().setCertification(data.certification.value);
    }
    if (data.released.hasValue()) {
        tvShow().setFirstAired(data.released.value);
    }
    if (data.runtime.hasValue()) {
        tvShow().setRuntime(data.runtime.value);
    }
    for (const Rating& rating : data.ratings) {
        tvShow().ratings().addRating(rating);
    }
    for (const QString& genre : data.genres) {
        tvShow().addGenre(genre);
    }
    for (const QString& keyword : data.keywords) {
        tvShow().addTag(keyword);
    }
    for (const Actor& actor : data.actors) {
        tvShow().addActor(actor);
    }
    if (data.poster.hasValue()) {
        tvShow().addPoster(data.poster.value);
    }
    for (const Poster& backdrop : data.backdrops) {
        tvShow().addBackdrop(backdrop);
    }
    if (data.network.hasValue()) {
        tvShow().addNetwork(data.network.value);
    }
    if (data.isOngoing.hasValue()) {
        tvShow().setStatus(data.isOngoing.value ? "Continuing" : "Ended");
    }
}

} // namespace scraper
} // namespace mediaelch
