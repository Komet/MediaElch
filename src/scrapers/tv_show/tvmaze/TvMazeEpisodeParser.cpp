#include "scrapers/tv_show/tvmaze/TvMazeEpisodeParser.h"

#include "scrapers/tv_show/tvmaze/TvMazeApi.h"
#include "tv_shows/TvShowEpisode.h"

#include <QJsonArray>

namespace mediaelch {
namespace scraper {

void TvMazeEpisodeParser::parseEpisode(TvShowEpisode& episode, const QJsonDocument& json)
{
    TvMazeEpisodeParser::parseEpisode(episode, json.object());
}

void TvMazeEpisodeParser::parseEpisode(TvShowEpisode& episode, const QJsonObject& json)
{
    if (json.isEmpty()) {
        return;
    }

    const int id = json["id"].toInt(-1);
    const int seasonNumber = json["season"].toInt(-1);
    const int episodeNumber = json["number"].toInt(-1);
    if (episodeNumber < 0 || seasonNumber < 0 || id < 0) {
        // If these details are not set then we can't properly scrape the episode.
        return;
    }

    episode.setTvMazeId(TvMazeId(QString::number(id)));
    episode.setEpisode(EpisodeNumber(episodeNumber));
    episode.setSeason(SeasonNumber(seasonNumber));
    episode.setTitle(json["name"].toString());
    episode.setOverview(TvMazeApi::removeBasicHtmlElements(json["summary"].toString()));
    episode.setFirstAired(QDate::fromString(json["airdate"].toString(), "yyyy-MM-dd"));

    // -------------------------------------

    QString thumbOriginal = json["image"].toObject()["original"].toString();
    if (!thumbOriginal.isEmpty() && thumbOriginal.startsWith("http")) {
        episode.setThumbnail(QUrl(thumbOriginal));
    }

    // TODO: Support "runtime" when TvShowEpisode supports it.
}

void TvMazeEpisodeParser::parseEpisodeFromOverview(TvShowEpisode& episode, const QJsonDocument& json)
{
    QJsonArray episodes = json.array();

    const int episodeNumber = episode.episodeNumber().toInt();
    const int seasonNumber = episode.seasonNumber().toInt();

    for (const QJsonValue& val : asConst(episodes)) {
        QJsonObject episodeObj = val.toObject();
        // Use default value -2 because -1 has special meaning.
        if (episodeObj["season"].toInt(-2) == seasonNumber && episodeObj["number"].toInt(-2) == episodeNumber) {
            TvMazeEpisodeParser::parseEpisode(episode, episodeObj);
            break;
        }
    }
}

} // namespace scraper
} // namespace mediaelch
