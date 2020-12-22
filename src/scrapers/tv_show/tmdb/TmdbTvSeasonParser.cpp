#include "scrapers/tv_show/tmdb/TmdbTvSeasonParser.h"

#include "globals/Helper.h"
#include "scrapers/tv_show/tmdb/TmdbTvEpisodeParser.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace mediaelch {
namespace scraper {

void TmdbTvSeasonParser::parseEpisodes(TmdbApi& api,
    const QJsonDocument& json,
    QObject* parentForEpisodes,
    std::function<void(TvShowEpisode*)> episodeCallback)
{
    const auto parsedJson = json.object();
    const auto episodesArray = parsedJson.value("episodes").toArray();

    for (const auto& episodeValue : episodesArray) {
        const QJsonObject episodeObj = episodeValue.toObject();
        auto* episode = new TvShowEpisode({}, parentForEpisodes);
        TmdbTvEpisodeParser::parseInfos(api, *episode, episodeObj);
        if (episode->seasonNumber() == SeasonNumber::NoSeason || episode->episodeNumber() == EpisodeNumber::NoEpisode) {
            episode->deleteLater();
        } else {
            episodeCallback(episode);
        }
    }
}

} // namespace scraper
} // namespace mediaelch
