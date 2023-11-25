#include "scrapers/tv_show/tmdb/TmdbTvSeasonParser.h"

#include "data/tv_show/TvShowEpisode.h"
#include "globals/Helper.h"
#include "scrapers/tv_show/tmdb/TmdbTvEpisodeParser.h"

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

QVector<Actor> TmdbTvSeasonParser::parseSeasonActors(TmdbApi& api, const QJsonDocument& data)
{
    if (data.isEmpty()) {
        return {};
    }

    QVector<Actor> actors{};

    const QJsonObject credits = data["credits"].toObject();
    const QJsonArray cast = credits["cast"].toArray();

    for (const QJsonValue& val : cast) {
        QJsonObject actorObj = val.toObject();
        Actor actor;
        actor.name = actorObj["name"].toString();
        actor.role = actorObj["character"].toString();
        actor.id = QString::number(actorObj["id"].toInt(-1));
        if (!actorObj["profile_path"].toString().isEmpty()) {
            actor.thumb = api.makeImageUrl(actorObj["profile_path"].toString()).toString();
        }
        actors.push_back(std::move(actor));
    }

    return actors;
}

} // namespace scraper
} // namespace mediaelch
