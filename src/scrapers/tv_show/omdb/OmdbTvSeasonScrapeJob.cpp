#include "scrapers/tv_show/omdb/OmdbTvSeasonScrapeJob.h"

#include "data/tv_show/TvShowEpisode.h"
#include "scrapers/omdb/OmdbApi.h"

#include <QJsonArray>
#include <QJsonObject>

namespace mediaelch {
namespace scraper {

OmdbTvSeasonScrapeJob::OmdbTvSeasonScrapeJob(OmdbApi& api, SeasonScrapeJob::Config _config, QObject* parent) :
    SeasonScrapeJob(std::move(_config), parent), m_api{api}
{
}

void OmdbTvSeasonScrapeJob::doStart()
{
    const QString& id = config().showIdentifier.str();

    if (!ImdbId::isValidFormat(id)) {
        ScraperError error;
        error.error = ScraperError::Type::ConfigError;
        error.message = tr("OMDb requires an IMDb ID for loading season details");
        setScraperError(error);
        emitFinished();
        return;
    }

    m_seasonsLeft = config().seasons;

    if (m_seasonsLeft.isEmpty()) {
        emitFinished();
        return;
    }

    for (SeasonNumber season : config().seasons) {
        loadSeason(season);
    }
}

void OmdbTvSeasonScrapeJob::loadSeason(SeasonNumber season)
{
    const ImdbId showId(config().showIdentifier.str());

    m_api.loadSeason(showId, season.toInt(), [this, season](QJsonDocument json, ScraperError error) {
        if (!error.hasError()) {
            QJsonArray episodes = json.object().value("Episodes").toArray();
            for (const QJsonValue& val : episodes) {
                QJsonObject obj = val.toObject();

                EpisodeNumber epNum(obj.value("Episode").toString().toInt());
                auto* episode = new TvShowEpisode({}, this);
                const auto& details = config().details;

                episode->setSeason(season);
                episode->setEpisode(epNum);

                if (details.contains(EpisodeScraperInfo::Title)) {
                    episode->setTitle(obj.value("Title").toString());
                }
                if (details.contains(EpisodeScraperInfo::FirstAired)) {
                    QDate date = QDate::fromString(obj.value("Released").toString(), "yyyy-MM-dd");
                    if (date.isValid()) {
                        episode->setFirstAired(date);
                    }
                }
                if (details.contains(EpisodeScraperInfo::Rating)) {
                    const QString ratingStr = obj.value("imdbRating").toString();
                    if (ratingStr != "N/A") {
                        Rating rating;
                        rating.source = "imdb";
                        rating.rating = ratingStr.toDouble();
                        rating.maxRating = 10.0;
                        episode->ratings().setOrAddRating(rating);
                    }
                }

                m_episodes[{season, epNum}] = episode;
            }
        }

        m_seasonsLeft.remove(season);
        onSeasonLoaded();
    });
}

void OmdbTvSeasonScrapeJob::onSeasonLoaded()
{
    if (m_seasonsLeft.isEmpty()) {
        emitFinished();
    }
}

} // namespace scraper
} // namespace mediaelch
